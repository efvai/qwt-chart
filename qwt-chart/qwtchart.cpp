#include "qwtchart.h"

#include "qwtchartplot.h"

#include <QLayout>
#include <QKeyEvent>

#include  <QwtPlotCanvas>

QwtChart::QwtChart(QWidget *parent)
    : QWidget(parent)
{
    QwtPlotCanvas *canvas = new QwtPlotCanvas();

    m_plot = new QwtChartPlot(canvas);

    QLayout *l = new QVBoxLayout();
    l->addWidget(m_plot);
    setLayout(l);

}

QwtChart::~QwtChart()
{
}

void QwtChart::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Space) {
        static int x_min = 0;
        static int x_max = 1;

        m_plot->setAxisScale(QwtAxis::XBottom, ++x_min, ++x_max);
        m_plot->replot();

    }
    QWidget::keyPressEvent(event);
}

