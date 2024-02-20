#ifndef QWTCHARTPLOT_H
#define QWTCHARTPLOT_H

#include <QwtPlot>
#include <QwtInterval>

class QwtChartScrollBar;

class QwtChartPlot : public QwtPlot
{
    Q_OBJECT
public:
    QwtChartPlot(QWidget *canvas);

public:
    bool eventFilter(QObject *, QEvent *) override;

public slots:
    void updateScrollBar();
    void layoutScrollBar(const QRect &rect);

protected slots:
    void scrollBarMoved(double min, double max);

protected:
    QwtChartScrollBar *m_scrollBar = nullptr;

private:
    QwtInterval m_dataInterval;
    QwtInterval m_visibleInterval;
    double m_visibleIntervalLength = 5.0;
};

#endif // QWTCHARTPLOT_H
