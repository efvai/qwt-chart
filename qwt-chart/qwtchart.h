#ifndef QWTCHART_H
#define QWTCHART_H

#include <QWidget>

class QwtChartPlot;

class QwtChart : public QWidget
{
    Q_OBJECT

public:
    QwtChart(QWidget *parent = nullptr);
    ~QwtChart();

protected:
    void keyPressEvent(QKeyEvent *event) override;

private:
    QwtChartPlot *m_plot = nullptr;
};
#endif // QWTCHART_H
