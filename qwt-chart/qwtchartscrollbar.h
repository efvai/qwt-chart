#ifndef QWTCHARTSCROLLBAR_H
#define QWTCHARTSCROLLBAR_H

#include <QScrollBar>

class QwtChartScrollBar : public QScrollBar
{
    Q_OBJECT
public:
    QwtChartScrollBar(QWidget *parent = nullptr);

    double minBaseValue() const;
    double maxBaseValue() const;

    double minSliderValue() const;
    double maxSliderValue() const;

    int extent() const;

signals:
    void sliderMovedCatch(double, double);
    void valueChangedCatch(double, double);

public slots:
    void setBase(double min, double max);
    void moveSlider(double min, double max);

protected:
    void sliderRange(int value, double &min, double &max) const;
    int mapToTick(double) const;
    double mapFromTick(double) const;

private:
    double m_minBase = 0.0;
    double m_maxBase = 1.0;
    int    m_baseTicks = 1e6;
};

#endif // QWTCHARTSCROLLBAR_H
