#ifndef QWTCHARTPLOT_H
#define QWTCHARTPLOT_H

#include <QwtPlot>

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
};

#endif // QWTCHARTPLOT_H
