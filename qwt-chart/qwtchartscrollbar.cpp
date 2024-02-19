#include "qwtchartscrollbar.h"

#include <QStyleOption>

QwtChartScrollBar::QwtChartScrollBar(QWidget *parent)
    : QScrollBar(Qt::Horizontal, parent)
{
    moveSlider(m_minBase, m_maxBase);
    connect(this, &QAbstractSlider::sliderMoved,
            this, [this] (int value) {
        double min, max;
        sliderRange(value, min, max);
        emit sliderMovedCatch(min, max);
    });
    connect(this, &QAbstractSlider::valueChanged,
            this, [this] (int value) {
        double min, max;
        sliderRange(value, min, max);
        emit valueChangedCatch(min, max);
    });
}

double QwtChartScrollBar::minBaseValue() const
{
    return m_minBase;
}

double QwtChartScrollBar::maxBaseValue() const
{
    return m_maxBase;
}

double QwtChartScrollBar::minSliderValue() const
{
    double min, dummy;
    sliderRange(value(), min, dummy);
    return min;
}

double QwtChartScrollBar::maxSliderValue() const
{
    double max, dummy;
    sliderRange(value(), dummy, max);
    return max;
}

int QwtChartScrollBar::extent() const
{
    QStyleOptionSlider opt;
    opt.initFrom(this);
    opt.subControls = QStyle::SC_None;
    opt.activeSubControls = QStyle::SC_None;
    opt.orientation = orientation();
    opt.minimum = minimum();
    opt.maximum = maximum();
    opt.sliderPosition = sliderPosition();
    opt.sliderValue = value();
    opt.singleStep = singleStep();
    opt.pageStep = pageStep();
    opt.upsideDown = invertedAppearance();

    if (orientation() == Qt::Horizontal)
        opt.state |= QStyle::State_Horizontal;

    return style()->pixelMetric(QStyle::PM_ScrollBarExtent, &opt, this);
}

void QwtChartScrollBar::setBase(double min, double max)
{
    if (min != m_minBase || max != m_maxBase) {
        m_minBase = min;
        m_maxBase = max;
        moveSlider(minSliderValue(), maxSliderValue());
    }
}

void QwtChartScrollBar::moveSlider(double min, double max)
{
    const int sliderTicks = qRound((max - min) /
        (m_maxBase - m_minBase) * m_baseTicks);

    // setRange initiates a valueChanged of the scrollbars
    // in some situations. So we block
    // and unblock the signals.
    blockSignals(true);

    setRange(sliderTicks / 2, m_baseTicks - sliderTicks / 2);
    int steps = sliderTicks / 200;
    if ( steps <= 0 )
        steps = 1;

    setSingleStep(steps);
    setPageStep(sliderTicks);

    int tick = mapToTick(min + (max - min) / 2);

    setSliderPosition(tick);
    blockSignals(false);
}

void QwtChartScrollBar::sliderRange(int value, double &min, double &max) const
{
    const int visibleTicks = pageStep();
    min = mapFromTick(value - visibleTicks / 2);
    max = mapFromTick(value + visibleTicks / 2);
}

int QwtChartScrollBar::mapToTick(double v) const
{
    const double pos = (v - m_minBase) /
            (m_maxBase - m_minBase) * m_baseTicks;
    return static_cast<int>(pos);
}

double QwtChartScrollBar::mapFromTick(double tick) const
{
    return m_minBase + (m_maxBase - m_minBase) * tick / m_baseTicks;
}
