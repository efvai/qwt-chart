#include "qwtchartplot.h"

#include "qwtchartscrollbar.h"

#include <QDebug>
#include <QEvent>
#include <QResizeEvent>

#include <QwtPlotGrid>
#include <QwtPlotLayout>
#include <QwtScaleWidget>
#include <QwtInterval>

QwtChartPlot::QwtChartPlot(QWidget *canvas)
    : QwtPlot(nullptr)
{
    setCanvas(canvas);
    setCanvasBackground(Qt::white);
    setAxisScale(QwtAxis::YLeft, 0.0, 10.0);
    setAxisScale(QwtAxis::XBottom, 0.0, 1.0);

    QwtPlotGrid *grid = new QwtPlotGrid();
    grid->attach(this);

    m_scrollBar = new QwtChartScrollBar(canvas);

    m_scrollBar->hide();
    connect(m_scrollBar, &QwtChartScrollBar::valueChangedCatch,
            this, &QwtChartPlot::scrollBarMoved);
    connect(axisWidget(QwtAxis::XBottom), SIGNAL(scaleDivChanged()),
            this, SLOT(updateScrollBar()));

    resize(600, 400);
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
    if (axisInterval(QwtAxis::XBottom).minValue() > 0) {
        m_scrollBar->setBase(0, axisInterval(QwtAxis::XBottom).maxValue());
        m_scrollBar->moveSlider(axisInterval(QwtAxis::XBottom).minValue(),
                                axisInterval(QwtAxis::XBottom).maxValue());
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
    setAxisScale(QwtAxis::XBottom, min, min + m_scrollBar->maxBaseValue());
    replot();
    axisWidget(QwtAxis::XBottom)->blockSignals(false);
}
