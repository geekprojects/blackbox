//
// Created by Ian Parker on 20/12/2025.
//

#ifndef BLACKBOX_POLYLINE_H
#define BLACKBOX_POLYLINE_H

#include <QGeoView/QGVDrawItem.h>

#include "routemap.h"

struct Flight;

struct Point
{
    QGV::GeoPos position;
    QPointF projected;
};

template <typename P> class PolyLine : public QGVDrawItem
{
 protected:
    FlightMap* m_map = nullptr;
    std::shared_ptr<Flight> m_flight;
    std::vector<QGVItem*> m_items;

    QGV::GeoRect m_boundingRect;
    QRectF m_boundingRectProjected;

    std::vector<P> m_points;

    PolyLine(FlightMap* map, std::shared_ptr<Flight> flight) : m_map(map), m_flight(flight) {}

    void updateBoundingRect()
    {
        if (m_points.empty())
        {
            return;
        }
        m_boundingRect = QGV::GeoRect(m_points[0].position, m_points[0].position);

        double minLat = m_points[0].position.latitude();
        double maxLat = m_points[0].position.latitude();
        double minLon = m_points[0].position.longitude();
        double maxLon = m_points[0].position.longitude();

        for (const auto& point : m_points)
        {
            if (point.position.latitude() < minLat)
            {
                minLat = point.position.latitude();
            }
            if (point.position.latitude() > maxLat)
            {
                maxLat = point.position.latitude();
            }
            if (point.position.longitude() < minLon)
            {
                minLon = point.position.longitude();
            }
            if (point.position.longitude() > maxLon)
            {
                maxLon = point.position.longitude();
            }
            /*
            if (point.altitude > m_maxAltitude)
            {
                m_maxAltitude = point.altitude;
            }
            */
        }
        m_boundingRect = QGV::GeoRect(
            QGV::GeoPos(minLat, minLon),
            QGV::GeoPos(maxLat, maxLon));
    }


 public:

    QPointF projAnchor() const override
    {
        return m_boundingRectProjected.center();
    }

    void onProjection(QGVMap* geoMap) override
    {
        QGVDrawItem::onProjection(geoMap);
        for (auto& point : m_points)
        {
            point.projected = geoMap->getProjection()->geoToProj(point.position);
        }

        m_boundingRectProjected = QRectF(
            geoMap->getProjection()->geoToProj(m_boundingRect.topLeft()),
            geoMap->getProjection()->geoToProj(m_boundingRect.bottomRight()));
    }


    QPainterPath projShape() const override
    {
        QPainterPath path;

        if (!m_points.empty())
        {
            path.moveTo(m_points.front().projected);
            for (auto it = m_points.begin(); it != m_points.end(); ++it)
            {
                if (it != m_points.begin())
                {
                    path.lineTo(it->projected);
                }
            }
        }

        return path;
    }

    QGV::GeoRect getRect() const
    {
        return m_boundingRect;
    }

    std::shared_ptr<Flight> getFlight() const { return m_flight; }

    virtual void removeFromMap()
    {
        for (auto item : m_items)
        {
            m_map->removeItem(item);
            delete item;
        }
        m_items.clear();
    }
};

#endif //BLACKBOX_POLYLINE_H
