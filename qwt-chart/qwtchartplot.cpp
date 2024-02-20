#include "qwtchartplot.h"

#include "qwtchartscrollbar.h"
#include "qwtchartmagnifier.h"

#include <QDebug>
#include <QEvent>
#include <QResizeEvent>
#include <QTimer>
#include <QtMath>

#include <QwtPlotGrid>
#include <QwtPlotLayout>
#include <QwtScaleWidget>
#include <QwtInterval>
#include <QwtPlotMagnifier>
#include <QwtPlotCurve>
#include <QwtPlotScaleItem>

namespace {
    class CurveData : public QwtSeriesData<QPointF>
    {
    public:
        virtual QPointF sample(size_t index) const QWT_OVERRIDE {
            return m_data.at(index);
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

    private:
        QList<QPointF> m_data;
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

    m_dataInterval = QwtInterval(0, 5);
    m_visibleInterval = QwtInterval(0, 5);
    setAxisScale(QwtAxis::XBottom, m_visibleInterval.minValue(),
                    m_visibleInterval.maxValue());

    static QwtPlotScaleItem *scaleItem = new QwtPlotScaleItem( QwtScaleDraw::RightScale, 2 );
    scaleItem->setFont( this->axisWidget( QwtAxis::YLeft )->font() );
    scaleItem->attach(this);

    this->setAxisVisible( QwtAxis::YLeft, false );

    static QwtPlotCurve *curve = new QwtPlotCurve;
    curve->setData(new CurveData);
    curve->setRenderHint(QwtPlotItem::RenderAntialiased);
    curve->setPaintAttribute(QwtPlotCurve::FilterPoints);
    curve->setPen(Qt::blue, 2);
    curve->attach(this);

    QTimer *timer = new QTimer;
    connect(timer, &QTimer::timeout,
            this, [&]() {
        static float dx = 0;
        if (dx < 25) {
            QPointF p(dx, 3.0 * qSin(dx * 5));
            dx += 0.01;
            CurveData *curveData = static_cast<CurveData*>(curve->data());
            curveData->appendData(p);
            m_dataInterval.setMaxValue(dx);
            if (dx > m_visibleIntervalLength) {
                m_visibleInterval.setMaxValue(dx);
                m_visibleInterval.setMinValue(dx - m_visibleIntervalLength);
            }
            setAxisScale(QwtAxis::XBottom, m_visibleInterval.minValue(), m_visibleInterval.maxValue());
            scaleItem->setPosition((m_visibleInterval.minValue() + m_visibleInterval.maxValue())*0.5);
            replot();
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
    if (m_visibleInterval.minValue() > 0) {
        m_scrollBar->setBase(m_dataInterval.minValue(),
                             m_dataInterval.maxValue());
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

void QwtChartPlot::scrollBarMoved(double min, double max)
{
    axisWidget(QwtAxis::XBottom)->blockSignals(true);
    setAxisScale(QwtAxis::XBottom, min, max);
    replot();
    axisWidget(QwtAxis::XBottom)->blockSignals(false);
}
