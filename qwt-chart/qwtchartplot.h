#ifndef QWTCHARTPLOT_H
#define QWTCHARTPLOT_H

#include <QwtPlot>
#include <QwtInterval>

class QwtPlotCurve;

class QwtChartScrollBar;

class QwtChartPlot : public QwtPlot
{
    Q_OBJECT
public:
    QwtChartPlot(QWidget *canvas);

public:
    bool eventFilter(QObject *, QEvent *) override;

    void setVisibleIntervalLength(double newVisibleIntervalLength);
    double visibleIntervalLength() const;

    void setVisibleInterval(double minVal, double maxVal);
public:
    void updateInterval();

    bool start() const;
    void setStart(bool newStart);

    QwtInterval visibleInterval() const;

    QwtInterval dataInterval() const;

public slots:
    void updateScrollBar();
    void layoutScrollBar(const QRect &rect);

    void setCurveResolution(int res);
    void setCurveOffset(int off);

protected slots:
    void scrollBarMoved(double min, double max);

protected:
    QwtChartScrollBar *m_scrollBar = nullptr;

private:
    QwtInterval m_dataInterval;
    QwtInterval m_visibleInterval;

    const double c_startIntervalLength = 5.0;
    double m_visibleIntervalLength = c_startIntervalLength;
    bool m_scrollBarNeeded = false;

    QwtPlotCurve *m_curve = nullptr;

    bool m_start = false;
};

#endif // QWTCHARTPLOT_H
