#include "qwtchartplot.h"

#include "qwtchartscrollbar.h"
#include "qwtchartmagnifier.h"

#include <QDebug>
#include <QEvent>
#include <QResizeEvent>

#include <QwtPlotGrid>
#include <QwtPlotLayout>
#include <QwtScaleWidget>
#include <QwtInterval>
#include <QwtPlotMagnifier>
#include <QwtPlotCurve>

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

    QVector<QPointF> points;
    for (int i = 0; i < 100; i++) {
        points.append(QPointF(i, i%4));
    }
    QwtPlotCurve *curve = new QwtPlotCurve();
    curve->setPen(Qt::blue, 3);
    curve->setRenderHint(QwtPlotItem::RenderAntialiased, true);
    curve->setSamples(points);
    curve->attach(this);

    m_dataInterval = QwtInterval(points.first().x(), points.last().x());
    m_visibleInterval = QwtInterval(points.last().x()
                                        - m_visibleIntervalLength, points.last().x());
    setAxisScale(QwtAxis::XBottom, m_visibleInterval.minValue(),
                    m_visibleInterval.maxValue());

    updateScrollBar();
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
    qDebug() << "interval: " <<axisInterval(QwtAxis::XBottom).width();
    m_visibleInterval = axisInterval(QwtAxis::XBottom);
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
