#include "qwtchart.h"

#include "qwtchartplot.h"

#include <QLayout>
#include <QKeyEvent>

#include <QwtPlotCanvas>

QwtChart::QwtChart(QWidget *parent)
    : QWidget(parent)
{
    QwtPlotCanvas *canvas = new QwtPlotCanvas();

    m_plot = new QwtChartPlot(canvas);

    QLayout *l = new QVBoxLayout();
    l->addWidget(m_plot);
    setLayout(l);

    resize(600, 400);
}

QwtChart::~QwtChart()
{
}

void QwtChart::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Space) {
        m_plot->setStart(!m_plot->start());
    }
    QWidget::keyPressEvent(event);
}

