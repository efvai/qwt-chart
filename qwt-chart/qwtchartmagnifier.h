#ifndef QWTCHARTMAGNIFIER_H
#define QWTCHARTMAGNIFIER_H

#include <QwtPlotMagnifier>

class QwtChartMagnifier : public QwtPlotMagnifier
{
    Q_OBJECT
public:
    QwtChartMagnifier(QWidget *canvas);

public slots:
    void rescale(double factor) override;

private:
    double findFactor(double interval, double sign);

private:
    const int c_magLevelLimitDown = -5;
    const int c_magLevelLimitUp   =  5;
    int m_currentMagLevel = 0;
};

#endif // QWTCHARTMAGNIFIER_H
