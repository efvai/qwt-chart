#include "qwtchartmagnifier.h"

#include "qwtchartplot.h"

#include <QDebug>
#include <QtMath>

#include <QwtPlot>
#include <QwtInterval>

QwtChartMagnifier::QwtChartMagnifier(QWidget *canvas)
    : QwtPlotMagnifier(canvas)
{
    setAxisEnabled(QwtAxis::YLeft, false);
    setZoomInKey(Qt::Key_Plus, Qt::KeypadModifier | Qt::ShiftModifier);
    setZoomOutKey(Qt::Key_Minus, Qt::KeypadModifier | Qt::ShiftModifier);
    setWheelFactor(0.0);
    setMouseFactor(0.0);
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

    QwtChartPlot *chart = static_cast<QwtChartPlot*>(plot());

    if ((width_2 * 2) > chart->dataInterval().width() * 1.2
        && factor > 1.0) {
        qInfo() << "Out of Range " << (width_2 * 2)
                << " > " << chart->dataInterval().width() * 1.5;
        return;
    }

    chart->setVisibleIntervalLength(width_2 * 2);
    if (center - width_2 > 0) {
        chart->setVisibleInterval(center - width_2,
                                  center + width_2);
    } else {
        chart->setVisibleInterval(0, center + width_2 * 2);
    }

    plot()->setAxisScale(QwtAxis::XBottom,
                         chart->visibleInterval().minValue(),
                         chart->visibleInterval().maxValue());
    m_currentMagLevel += inc;
    plot()->replot();
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
