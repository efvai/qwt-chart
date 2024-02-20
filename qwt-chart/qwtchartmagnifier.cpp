#include "qwtchartmagnifier.h"

#include <QDebug>
#include <QtMath>

#include <QwtPlot>
#include <QwtInterval>

QwtChartMagnifier::QwtChartMagnifier(QWidget *canvas)
    : QwtPlotMagnifier(canvas)
{
    setAxisEnabled(QwtAxis::YLeft, false);
    setZoomInKey(Qt::Key_Plus, Qt::KeypadModifier);
    setZoomOutKey(Qt::Key_Minus, Qt::KeypadModifier);
    setKeyFactor(0.75);
}

void QwtChartMagnifier::rescale(double factor)
{
    int inc = 0;
    factor > 1.0 ? inc++ : inc--;
    if (m_currentMagLevel + inc > c_magLevelLimitUp
        || m_currentMagLevel + inc < c_magLevelLimitDown) {
        qInfo() << "Limit: " << m_currentMagLevel;
        return;
    }

    QwtInterval interval = plot()->axisInterval(QwtAxis::XBottom);
    double center = 0.5 * (interval.maxValue() + interval.minValue());
    double width_2 = 0.5 * interval.width() * factor;

    if (center - width_2 > 0) {
        plot()->setAxisScale(QwtAxis::XBottom, center - width_2, center + width_2);
        plot()->replot();
        m_currentMagLevel += inc;
    }
    // double newFactor = findFactor(
    //     plot()->axisInterval(QwtAxis::XBottom).width(), factor);
    // if (newFactor == 1.0) {
    //     newFactor = factor;
    // }
    // qDebug() << "Factor: " << newFactor;
    // setKeyFactor(newFactor);

    // QwtPlotMagnifier::rescale(newFactor);
    // m_currentMagLevel += inc;

    // if (plot()->axisInterval(QwtPlot::xBottom).minValue() < 0) {
    //     QwtPlotMagnifier::rescale(1 / newFactor);
    //     m_currentMagLevel -= inc;
    // }



}

double QwtChartMagnifier::findFactor(double interval, double sign)
{
    const double posFactors[3] {2.5, 2.0, 2.0};
    const double negFactors[3] {0.5, 0.4, 0.5};
    const double dimensions[3] {1.0 * qPow(10, c_magLevelLimitUp),
                                2.5 * qPow(10, c_magLevelLimitUp),
                                5.0 * qPow(10, c_magLevelLimitUp)};

    for (int i = 0; i < 3; i++) {
        double f = interval;
        while (f <= dimensions[i]) {
            if (qRound(f) == dimensions[i]) {
                if (sign > 1.0) {
                    return posFactors[i];
                } else {
                    return negFactors[i];
                }
            }
            f *= 10;
        }
    }

    return 1.0;
}
