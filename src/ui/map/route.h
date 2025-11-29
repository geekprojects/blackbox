//
// Created by Ian Parker on 14/11/2025.
//

#ifndef BLACKBOX_ROUTE_H
#define BLACKBOX_ROUTE_H

#include <QGeoView/QGVDrawItem.h>

#include <QBrush>

#include "routemap.h"
#include "blackbox/state.h"

struct Point
{
    QGV::GeoPos position;
    float altitude;
    float heading;

    QPointF projected;
};

class Route :  public QGVDrawItem
{
    RouteMap* m_map = nullptr;
    uint64_t m_flightId;

    std::vector<Point> m_points;
    QGV::GeoRect m_boundingRect;
    QRectF m_boundingRectProjected;

    float m_maxAltitude = 0;

    State m_lastState;
    uint64_t m_lastTimestamp = 0;
    std::vector<QGVItem*> m_items;

    QImage* m_planeIcon = nullptr;
    QGVIcon* m_positionIcon = nullptr;

    QTimer* m_updateTimer;

    void onProjection(QGVMap* geoMap) override;
    QPainterPath projShape() const override;
    void projPaint(QPainter* painter) override;
    QPointF projAnchor() const override;
    QTransform projTransform() const override;
    QString projTooltip(const QPointF& projPos) const override;
    void projOnMouseClick(const QPointF& projPos) override;

public:
    Route(RouteMap* map, uint64_t flightId);

    //void set(std::vector<Point> points);
    void addPoints(std::vector<Point> points);
    void clear();

    QGV::GeoRect getRect() const;

    Point getLastPosition();

    void updateRoute();

    void showRoute();

    uint64_t getFlight() const { return m_flightId; }

    void removeFromMap();
};


#endif //BLACKBOX_ROUTE_H
