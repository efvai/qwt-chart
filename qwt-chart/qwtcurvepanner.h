#ifndef QWTCURVEPANNER_H
#define QWTCURVEPANNER_H

#include <QObject>
#include <QPointF>
#include <QPointer>

class QwtPlot;
class QwtPlotMarker;
class QwtWidgetOverlay;

class QPainter;
class QRegion;

class QwtCurvePanner : public QObject
{
    Q_OBJECT
public:
    QwtCurvePanner(QwtPlot *plot);
    virtual ~QwtCurvePanner();

public:
    void drawOverlay(QPainter *) const;
    QRegion maskHint() const;

    bool eventFilter(QObject *object, QEvent *event) override;

private:
    bool pressed(const QPointF &p);
    bool moved(const QPointF &p);
    bool released(const QPointF &p);

    QwtPlotMarker* itemAt(const QPointF &p) const;
    void setItemVisible(QwtPlotMarker *item, bool on);

private:
    QwtPlot *m_plot = nullptr;
    QwtPlotMarker *m_panningItem = nullptr;
    QPointF m_currentPos;

    QPointer<QwtWidgetOverlay> m_overlay;
};

#endif // QWTCURVEPANNER_H
