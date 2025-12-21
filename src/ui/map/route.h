//
// Created by Ian Parker on 16/12/2025.
//

#ifndef BLACKBOX_ROUTE_H
#define BLACKBOX_ROUTE_H

#include <QGeoView/QGVDrawItem.h>

#include "polyline.h"
#include "routemap.h"
#include "../navigraph.h"
#include "blackbox/datastore.h"

class RouteMarker;


struct RoutePoint : Point
{
    std::string name;
    std::string airway;

    RouteMarker* marker = nullptr;
};

class Route : public PolyLine<RoutePoint>
{
    std::shared_ptr<NavigraphData> m_navigraphData;

    bool parseLatLon(std::string ident, UFC::Coordinate &position);

 public:
    Route(std::shared_ptr<NavigraphData> navigraphData, FlightMap* map, std::shared_ptr<Flight> flightId);

    bool parseRoute();

    void projPaint(QPainter* painter) override;
};

#endif //BLACKBOX_ROUTE_H
