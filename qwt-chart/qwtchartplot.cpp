#include "qwtchartplot.h"

#include "qwtchartscrollbar.h"
#include "qwtchartmagnifier.h"
#include "qwtcurvepanner.h"

#include <QDebug>
#include <QEvent>
#include <QResizeEvent>
#include <QTimer>
#include <QtMath>
#include <QPainterPath>

#include <QwtPlotGrid>
#include <QwtPlotLayout>
#include <QwtScaleWidget>
#include <QwtInterval>
#include <QwtPlotMagnifier>
#include <QwtPlotCurve>
#include <QwtPlotScaleItem>
#include <QwtPanner>
#include <QwtPlotMarker>
#include <QwtSymbol>

namespace {
class ArrowSymbol : public QwtSymbol
{
public:
    ArrowSymbol()
    {
        QPen pen( Qt::black, 0 );
        pen.setJoinStyle( Qt::MiterJoin );

        setPen( pen );
        setBrush( Qt::red );

        QPainterPath path;
        path.moveTo( 0, 5 );
        path.lineTo( -3, 5 );
        path.lineTo( 0, 0 );
        path.lineTo( 3, 5 );
        path.lineTo( 0, 5 );

        QTransform transform;
        transform.rotate( 90.0 );
        path = transform.map( path );

        setPath( path );
        setPinPoint( QPointF( 0, 0 ) );

        setSize( 15, 19 );
    }
};

class CurveData : public QwtSeriesData<QPointF>
{
public:
    virtual QPointF sample(size_t index) const QWT_OVERRIDE {
        QPointF p = m_data.at(index);
        p.setY((p.y() +  m_yOffset) * m_resolution);
        return p;
    }

    virtual size_t size() const QWT_OVERRIDE {
        return m_data.size();
    }

    virtual QRectF boundingRect() const QWT_OVERRIDE {
        return m_boundingRect;
    }


    CurveData() {
        m_data.reserve(1000);
        m_boundingRect = QRectF(1.0, 1.0, -2.0, -2.0);
    }

    void appendData(const QPointF &point) {
        m_data.append(point);
        if (m_boundingRect.width() < 0 || m_boundingRect.height() < 0) {
            m_boundingRect.setRect(point.x(), point.y(), 0.0, 0.0);
        } else {
            m_boundingRect.setRight(point.x());
            if (point.y() > m_boundingRect.bottom()) {
                m_boundingRect.setBottom(point.y());
            }
            if (point.y() < m_boundingRect.top()) {
                m_boundingRect.setTop(point.y());
            }
        }
    }

    void magnifyData(int res) {
        if (res == 1) {
            m_resolution += 0.1;
        } else {
            m_resolution -= 0.1;
        }
    }

    void offsetData(int off) {
        m_yOffset += off;
    }

private:
    QList<QPointF> m_data;
    double m_resolution = 1.0;
    double m_yOffset = 0.0;
    QRectF m_boundingRect;

};
}

QwtChartPlot::QwtChartPlot(QWidget *canvas)
    : QwtPlot(nullptr)
{
    setCanvas(canvas);
    setCanvasBackground(Qt::white);
    setAxisScale(QwtAxis::YLeft,  -4.0, 4.0);


    QwtPlotGrid *grid = new QwtPlotGrid();
    grid->attach(this);

    m_scrollBar = new QwtChartScrollBar(canvas);

    m_scrollBar->hide();
    connect(m_scrollBar, &QwtChartScrollBar::valueChangedCatch,
            this, &QwtChartPlot::scrollBarMoved);
    connect(axisWidget(QwtAxis::XBottom), SIGNAL(scaleDivChanged()),
            this, SLOT(updateScrollBar()));

    (void) new QwtChartMagnifier(canvas);

    m_dataInterval = QwtInterval(0, c_startIntervalLength);
    m_visibleInterval = QwtInterval(0, c_startIntervalLength);
    setAxisScale(QwtAxis::XBottom, m_visibleInterval.minValue(),
                    m_visibleInterval.maxValue());

    static QwtPlotScaleItem *scaleItem = new QwtPlotScaleItem( QwtScaleDraw::RightScale, 2 );
    scaleItem->setFont(this->axisWidget( QwtAxis::YLeft )->font());
    scaleItem->attach(this);

    this->setAxisVisible( QwtAxis::YLeft, false );

    setAxisMaxMajor(QwtAxis::XBottom, 5);

    m_curve = new QwtPlotCurve;
    m_curve->setData(new CurveData);
    m_curve->setRenderHint(QwtPlotItem::RenderAntialiased);
    m_curve->setPaintAttribute(QwtPlotCurve::FilterPoints);
    m_curve->setPen(Qt::blue, 2);
    m_curve->attach(this);

    QwtPlotMarker* mPos = new QwtPlotMarker( "Marker" );
    mPos->setRenderHint( QwtPlotItem::RenderAntialiased, true );
    mPos->setItemAttribute( QwtPlotItem::Legend, true );
    mPos->setSymbol(new ArrowSymbol());
    mPos->setValue(QPointF( 0, 2.5 ));
    mPos->setLabel(QString( "%1  " ).arg( 1 ));
    mPos->setLabelAlignment( Qt::AlignLeft);
    mPos->attach( this );

    (void) new QwtCurvePanner(this);

    plotLayout()->setCanvasMargin(20, QwtAxis::YLeft);


    QTimer *timer = new QTimer;
    connect(timer, &QTimer::timeout,
            this, [&]() {
        static float dx = 0;
        if (m_start) {
            QPointF p(dx, 3.0 * qSin(dx * 5.0));
            dx += 0.01;
            CurveData *curveData = static_cast<CurveData*>(m_curve->data());
            curveData->appendData(p);
            m_dataInterval.setMaxValue(dx);
            updateInterval();
        }
    });
    timer->start(5);
    replot();
}

bool QwtChartPlot::eventFilter(QObject *object, QEvent *event)
{
    if (object == canvas()) {
        switch (event->type()) {
        case QEvent::Resize:
        {
            const QMargins m = canvas()->contentsMargins();
            QRect rect;
            rect.setSize(static_cast<QResizeEvent* >(event)->size() );
            rect.adjust(m.left(), m.top(), -m.right(), -m.bottom());

            layoutScrollBar(rect);
            break;
        }
        case QEvent::ChildRemoved:
        {
            const QObject* child =
                static_cast< QChildEvent* >(event)->child();
            if (child == m_scrollBar) {
                m_scrollBar = nullptr;
            }
            break;
        }
        case QEvent::KeyPress:
        {
            const QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
            if (keyEvent->key() == Qt::Key_Plus
                && keyEvent->modifiers() == Qt::KeypadModifier) {
                setCurveResolution(1);
            } else if (keyEvent->key() == Qt::Key_Minus
                       && keyEvent->modifiers() == Qt::KeypadModifier) {
                setCurveResolution(-1);
            }
            if (keyEvent->key() == Qt::Key_Plus
                && keyEvent->modifiers() == (Qt::KeypadModifier
                       | Qt::ControlModifier)) {
                setCurveOffset(1);
            } else if (keyEvent->key() == Qt::Key_Minus
                       && keyEvent->modifiers() == (Qt::KeypadModifier
                       | Qt::ControlModifier)) {
                setCurveOffset(-1);
            }
            break;
        }
        default:
            break;
        }
    }
    return QwtPlot::eventFilter(object, event);
}

void QwtChartPlot::updateScrollBar()
{
    if (!canvas()) {
        return;
    }
    const int xAxis = QwtAxis::XBottom;

    QwtPlotLayout *layout = plotLayout();

//    int start, end;
//    axisWidget(QwtAxis::XBottom)->getBorderDistHint(start, end);
//    axisWidget(QwtAxis::XBottom)->setMinBorderDist(start, end);

//    axisWidget(QwtAxis::YLeft)->getBorderDistHint(start, end);
//    axisWidget(QwtAxis::YLeft)->setMinBorderDist(start, end);

//    layout->setAlignCanvasToScales( false );

    // todo: check scrollbar necessary
    if (!m_scrollBarNeeded) {
        if (m_visibleInterval.maxValue() > m_visibleIntervalLength) {
            m_scrollBarNeeded = true;
        }
    }
    if (m_scrollBarNeeded) {
        double maxBase = 0.0;
        maxBase = (axisInterval(xAxis).maxValue() > m_dataInterval.maxValue()) ?
             axisInterval(xAxis).maxValue() : m_dataInterval.maxValue();
        m_scrollBar->setBase(0,
                             maxBase);
        m_scrollBar->moveSlider(m_visibleInterval.minValue(),
                                m_visibleInterval.maxValue());
        if (!m_scrollBar->isVisibleTo(canvas())) {
            m_scrollBar->setPalette(palette());
            m_scrollBar->show();
            layout->setCanvasMargin(layout->canvasMargin(xAxis)
                                    + m_scrollBar->extent(), xAxis);
        }
    } else {
        if (m_scrollBar) {
            m_scrollBar->hide();
            layout->setCanvasMargin( layout->canvasMargin(xAxis)
                - m_scrollBar->extent(), xAxis);
        }
    }
    layoutScrollBar(canvas()->contentsRect());
    updateLayout();
}

void QwtChartPlot::layoutScrollBar(const QRect &rect)
{
    const int hDim = m_scrollBar ? m_scrollBar->extent() : 0;

    if (m_scrollBar && m_scrollBar->isVisible()) {
        int x = rect.x();
        int y = rect.bottom() - hDim + 1;
        int w = rect.width();
        m_scrollBar->setGeometry(x, y, w, hDim);
    }
}

void QwtChartPlot::setCurveResolution(int res)
{
    CurveData *curveData = static_cast<CurveData*>(m_curve->data());
    curveData->magnifyData(res);
    replot();
}

void QwtChartPlot::setCurveOffset(int off)
{
    CurveData *curveData = static_cast<CurveData*>(m_curve->data());
    curveData->offsetData(off);
    replot();
}

void QwtChartPlot::scrollBarMoved(double min, double max)
{
    axisWidget(QwtAxis::XBottom)->blockSignals(true);
    setAxisScale(QwtAxis::XBottom, min, max);
    replot();
    axisWidget(QwtAxis::XBottom)->blockSignals(false);
}

QwtInterval QwtChartPlot::dataInterval() const
{
    return m_dataInterval;
}

QwtInterval QwtChartPlot::visibleInterval() const
{
    return m_visibleInterval;
}

bool QwtChartPlot::start() const
{
    return m_start;
}

void QwtChartPlot::setStart(bool newStart)
{
    m_start = newStart;
}

double QwtChartPlot::visibleIntervalLength() const
{
    return m_visibleIntervalLength;
}

void QwtChartPlot::setVisibleInterval(double minVal, double maxVal)
{
    m_visibleInterval.setInterval(minVal, maxVal);
}

void QwtChartPlot::updateInterval()
{
    //CurveData *curveData = static_cast<CurveData*>(m_curve->data());
    //double lastDataVal = curveData->boundingRect().right();
    double lastDataVal = m_dataInterval.maxValue();
    double minVisibleVal = lastDataVal - (m_visibleIntervalLength);
    if (minVisibleVal >= 0) {
        m_visibleInterval.setInterval(minVisibleVal, lastDataVal);
        setAxisScale(QwtAxis::XBottom, minVisibleVal, lastDataVal);
    } else {
        m_visibleInterval.setInterval(0, m_visibleIntervalLength);
        setAxisScale(QwtAxis::XBottom, 0, m_visibleIntervalLength);
    }
    replot();
}

void QwtChartPlot::setVisibleIntervalLength(double newVisibleIntervalLength)
{
    m_visibleIntervalLength = newVisibleIntervalLength;
}
