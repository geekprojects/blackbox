//
// Created by Ian Parker on 16/12/2025.
//

#include "route.h"

#include <qabstracttextdocumentlayout.h>
#include <QGeoView/QGVMapQGView.h>

#include <common/utils.h>

#include "routemarker.h"
#include "routeparser.h"

#include "../utils/greatcircle.h"

using namespace std;

Route::Route(shared_ptr<NavigraphData> navigraphData, FlightMap* map, shared_ptr<Flight> flightId) :
    PolyLine(map, flightId),
    m_navigraphData(navigraphData)
{
}


enum class RouteState {
    START,
    ENROUTE,
    END
};

bool Route::parseRoute()
{
    RouteParser routeParser(m_navigraphData);
    routeParser.createRoute(m_flight, m_points);

    if (m_points.size() < 2)
    {
        printf("parseRoute: Not enough points!\n");
        return false;
    }

    updateBoundingRect();
    printf("parseRoute: Parsed route:\n");
    for (auto& point : m_points)
    {
        printf("parseRoute: %s: %0.2f, %0.2f\n", point.name.c_str(), point.position.latitude(), point.position.longitude());

        if (!point.name.empty())
        {

        RouteMarker* marker = new RouteMarker(point);
        point.marker = marker;

        m_map->getItemsLayer()->addItem(marker);
        m_items.push_back(marker);

        marker->bringToFront();
        }
    }
    return true;
}

bool Route::createRoute()
{
    m_points.clear();
    string routeStr = m_flight->route;

    Airport originAirport;
    Airport destAirport;

    if (!m_flight->origin.empty())
    {
        bool foundAirport = m_navigraphData->findAirport(m_flight->origin, originAirport);
        if (!foundAirport)
        {
            printf("parseRoute: Invalid origin: %s\n", m_flight->origin.c_str());
        }
    }
    else
    {
        // No origin set??
        printf("parseRoute: No origin set! This may end badly\n");
        originAirport.code = m_flight->origin;
    }

    if (!m_flight->destination.empty())
    {
        bool foundAirport = m_navigraphData->findAirport(m_flight->destination, destAirport);
        if (!foundAirport)
        {
            printf("parseRoute: Invalid destination: %s\n", m_flight->destination.c_str());
        }
    }

    if (!parseRoute())
    {
        return false;
    }

    vector<RoutePoint> greatCircleRoute;
    for (auto it = m_points.begin(); it != m_points.end(); ++it)
    {
        RoutePoint& rp = *it;
        bool added = false;
        if (it != m_points.begin())
        {
            auto& prev = *(it - 1);
            auto distance = Utils::distance(prev.position.latitude(), prev.position.longitude(), rp.position.latitude(), rp.position.longitude());
            printf("parseRoute: Distance from %s to %s: %0.2f\n", prev.name.c_str(), rp.name.c_str(), distance);
            if (distance > 300.0f)
            {
                GreatCircle gc(prev.position, rp.position);
                auto points = gc.arc(distance / 100.0f);
                for (auto p : points)
                {
                    RoutePoint gcPoint;
                    gcPoint.name = rp.name;
                    gcPoint.position.setLat(p.latitude());
                    gcPoint.position.setLon(p.longitude());
                    greatCircleRoute.push_back(gcPoint);
                }
                added = true;
            }
        }
        if (!added)
        {
            greatCircleRoute.push_back(rp);
        }
        //greatCircleRoute.push_back(*it);
        //gc.arc(5);
    }
    m_points = greatCircleRoute;

    return true;
}

void Route::projPaint(QPainter* painter)
{
    if (!m_map->isVisible())
    {
        // updateRoute: Not visible! Not updating!
        return;
    }

    painter->setRenderHint(QPainter::Antialiasing, true);

    QPen pen;
    pen.setWidth(10);
    pen.setCosmetic(true);

    auto rect = m_map->geoView()->getCamera().projRect();

    QColor colour1;
    colour1 =  QColor(128, 128, 128);
    pen.setColor(colour1);
    painter->setPen(pen);
    painter->setBrush(QBrush(colour1));

    QFont font("Courier New");
    font.setPointSize(1000);
    painter->setFont(font);

    RoutePoint previous;
    for (auto it = m_points.begin(); it != m_points.end(); ++it)
    {
        QPointF const& p2 = it->projected;
        if (it != m_points.begin())
        {
            QPointF const& p1 = previous.projected;
            QRect const pr(p1.toPoint(), p2.toPoint());
            if (rect.intersects(pr))
            {
                painter->drawLine(p1, p2);
            }
        }
        painter->drawEllipse(p2, 50, 50);
        previous = *it;
    }
}
