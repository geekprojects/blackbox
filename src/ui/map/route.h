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

    NavAidType type;

    RouteMarker* marker = nullptr;

    uint64_t sourceId = 0;
};

class Route : public PolyLine<RoutePoint>
{
    std::shared_ptr<NavigraphData> m_navigraphData;

 public:
    Route(std::shared_ptr<NavigraphData> navigraphData, FlightMap* map, std::shared_ptr<Flight> flightId);

    bool parseRoute(
    );

    bool createRoute();

    void projPaint(QPainter* painter) override;
};

#endif //BLACKBOX_ROUTE_H
