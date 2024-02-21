#include "qwtcurvepanner.h"

#include <QwtPlot>
#include <QwtPlotCanvas>
#include <QwtPlotMarker>
#include <QwtScaleMap>
#include <QwtWidgetOverlay>

#include <QMouseEvent>
#include <QDebug>
#include <QPainter>

namespace {
class Overlay : public QwtWidgetOverlay
{
public:
    Overlay(QWidget *parent, QwtCurvePanner *panner)
        : QwtWidgetOverlay(parent), m_curvePanner(panner)
    {
        setMaskMode(QwtWidgetOverlay::MaskHint);
        setRenderMode(QwtWidgetOverlay::AutoRenderMode);
    }

protected:
    virtual void drawOverlay(QPainter *painter) const override
    {
        m_curvePanner->drawOverlay(painter);
    }

    virtual QRegion maskHint() const override
    {
        return m_curvePanner->maskHint();
    }

private:
    QwtCurvePanner *m_curvePanner = nullptr;
};
}

QwtCurvePanner::QwtCurvePanner(QwtPlot *plot)
    : QObject (plot), m_overlay(nullptr)
{
    m_plot = plot;

    m_plot->canvas()->installEventFilter(this);
}

QwtCurvePanner::~QwtCurvePanner()
{
    delete m_overlay;
}

void QwtCurvePanner::drawOverlay(QPainter *painter) const
{
    if (!m_plot || !m_panningItem) {
        return;
    }
    const QwtScaleMap xMap = m_plot->canvasMap(m_panningItem->xAxis());
    const QwtScaleMap yMap = m_plot->canvasMap(m_panningItem->yAxis());
    painter->setRenderHint( QPainter::Antialiasing,
                           m_panningItem->testRenderHint( QwtPlotItem::RenderAntialiased ) );
    m_panningItem->draw( painter, xMap, yMap,
                       m_plot->canvas()->contentsRect() );
}

QRegion QwtCurvePanner::maskHint() const
{
    // temp
    return m_plot->canvas()->mask();
}

bool QwtCurvePanner::eventFilter(QObject *object, QEvent *event)
{
    if (m_plot && object == m_plot->canvas()) {
        switch (event->type()) {
        case QEvent::MouseButtonPress: {
            const QMouseEvent *mouseEvent =
                dynamic_cast<QMouseEvent*>(event);
            if (!m_overlay || mouseEvent->button() == Qt::LeftButton) {
                const bool accepted = pressed(mouseEvent->pos());
                if (accepted) {
                    m_overlay = new Overlay(m_plot->canvas(), this);
                    m_overlay->updateOverlay();
                    m_overlay->show();
                }
            }
            break;
        }
        case QEvent::MouseMove: {
            if (m_overlay) {
                const QMouseEvent *mouseEvent =
                    dynamic_cast<QMouseEvent*>(event);
                const bool accepted = moved(mouseEvent->pos());
                if (accepted) {
                    m_overlay->updateOverlay();
                }
            }
            break;
        }
        case QEvent::MouseButtonRelease: {
            const QMouseEvent *mouseEvent =
                dynamic_cast<QMouseEvent*>(event);
            if (m_overlay && mouseEvent->button() == Qt::LeftButton) {
                    released(mouseEvent->pos());
                    delete m_overlay;
                    m_overlay = nullptr;
            }
            break;
        }
        default: {
            break;
        }
        }
        return false;
    }

    return QObject::eventFilter(object, event);
}

bool QwtCurvePanner::pressed(const QPointF &p)
{
    m_panningItem = itemAt(p);
    if (m_panningItem) {
        m_currentPos = p;
        setItemVisible(m_panningItem, false);
        return true;
    }
    return false;
}

bool QwtCurvePanner::moved(const QPointF &p)
{
    if (!m_plot || !m_panningItem) {
        return false;
    }
    const QwtScaleMap xMap = m_plot->canvasMap(m_panningItem->xAxis());
    const QwtScaleMap yMap = m_plot->canvasMap(m_panningItem->yAxis());

    //const QPointF p1 = QwtScaleMap::invTransform(xMap, yMap, m_currentPos);
    const QPointF p2 = QwtScaleMap::invTransform(xMap, yMap, p);

    qDebug() << "moved " << p;
    m_panningItem->setYValue(p2.y());

    m_currentPos = p;
    return true;
}

bool QwtCurvePanner::released(const QPointF &p)
{
    Q_UNUSED(p);
    qDebug() << "released " << m_panningItem;
    if (m_panningItem) {
        setItemVisible(m_panningItem, true);
    }
    return true;
}

QwtPlotMarker *QwtCurvePanner::itemAt(const QPointF &p) const
{
    using namespace QwtAxis;

    double coords[AxisPositions];

    coords[XBottom] = m_plot->canvasMap(XBottom).invTransform(p.x());
    coords[XTop] =    m_plot->canvasMap(XTop).invTransform(p.x());
    coords[YLeft] =   m_plot->canvasMap(YLeft).invTransform(p.y());
    coords[YRight] =  m_plot->canvasMap(YRight).invTransform(p.y());

    QwtPlotItemList items = m_plot->itemList();
    for (int i = items.size() - 1; i >= 0; i--)
    {
        QwtPlotItem* item = items[i];
        if (item->isVisible() &&
            item->rtti() == QwtPlotItem::Rtti_PlotMarker)
        {
            QwtPlotMarker* markerItem =
                static_cast<QwtPlotMarker*>( item );
            const QPointF point(coords[item->xAxis()], coords[item->yAxis()]);
            const QPointF marker = markerItem->value();
            float dist = ((point.x() - marker.x()) * (point.x() - marker.x()))
                         + ((point.y() - marker.y()) * (point.y() - marker.y()));

            if (dist < 0.04)
            {
                qDebug() << dist << " catched " << i;
                return markerItem;
            }
        }
    }

    return nullptr;
}

void QwtCurvePanner::setItemVisible(QwtPlotMarker *item, bool on)
{
    if (!m_plot || !item || item->isVisible() == on) {
        return;
    }
    item->setVisible(on);

    m_plot->canvas()->update();
    // QwtPlotCanvas *canvas =
    //     qobject_cast<QwtPlotCanvas*>(m_plot->canvas());
    // if (canvas) {
    //     canvas->invalidateBackingStore();
    // }
    // // mask hint ?
    // m_plot->canvas()->update();
}
