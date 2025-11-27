//
// Created by Ian Parker on 14/11/2025.
//

#ifndef BLACKBOX_ROUTE_H
#define BLACKBOX_ROUTE_H

#include <QGeoView/QGVDrawItem.h>

#include <QBrush>
#include <QPen>

struct Point
{
    QGV::GeoPos position;
    float altitude;
    float heading;

    QPointF projected;
};

class Route :  public QGVDrawItem
{
    void onProjection(QGVMap* geoMap) override;
    QPainterPath projShape() const override;
    void projPaint(QPainter* painter) override;
    QPointF projAnchor() const override;
    QTransform projTransform() const override;
    QString projTooltip(const QPointF& projPos) const override;
    void projOnMouseClick(const QPointF& projPos) override;
    void projOnMouseDoubleClick(const QPointF& projPos) override;
    void projOnObjectStartMove(const QPointF& projPos) override;
    void projOnObjectMovePos(const QPointF& projPos) override;
    void projOnObjectStopMove(const QPointF& projPos) override;

    std::vector<Point> m_points;
    QGV::GeoRect m_boundingRect;
    QRectF m_boundingRectProjected;
    QColor mColor;

    float m_maxAltitude = 0;

public:
    Route(QColor color);

    void set(std::vector<Point> points);
    void addPoints(std::vector<Point> points);
    void clear();

    QGV::GeoRect getRect() const;

    Point getLastPosition();
};


#endif //BLACKBOX_ROUTE_H
