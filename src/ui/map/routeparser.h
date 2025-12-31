//
// Created by Ian Parker on 26/12/2025.
//

#ifndef BLACKBOX_ROUTEPARSER_H
#define BLACKBOX_ROUTEPARSER_H

#include <memory>

#include "route.h"
#include "blackbox/datastore.h"
#include "ui/navigraph.h"

class RouteParser : public BlackBox::Logger
{
    std::shared_ptr<NavigraphData> m_navigraphData;

    bool parseRoute(
        const std::shared_ptr<Flight>& flight,
        Airport &originAirport,
        Airport &destAirport,
        UFC::Coordinate &lastCoord,
        std::vector<RoutePoint> &resolvedPoints);

    void expandAirways(std::vector<RoutePoint> &resolvedPoints, std::vector<std::shared_ptr<RoutePoint>> parsedPoints);

    static std::vector<RoutePoint> generateGreatCirclePaths(std::vector<RoutePoint> &points);
    static void generateGreatCirclePath(
        std::vector<RoutePoint> &greatCircleRoute,
        RoutePoint const &rp,
        RoutePoint const &prev,
        double distance);

    static void addAirport(const Airport &originAirport, std::vector<RoutePoint> &resolvedPoints);

    std::shared_ptr<RoutePoint> parseWaypoint(
         UFC::Coordinate &lastCoord,
         const std::string &ident,
         NavAid &navAid) const;

    void parseAirway(
        std::vector<std::string> route,
        std::vector<std::string>::iterator &it,
        const std::shared_ptr<RoutePoint> &rp);

 public:
    explicit RouteParser(const std::shared_ptr<NavigraphData> &navigraphData);

    bool createRoute(const std::shared_ptr<Flight>& flight, std::vector<RoutePoint> &points);



    static bool parseLatLon(const std::string &ident, UFC::Coordinate &position);
};


#endif //BLACKBOX_ROUTEPARSER_H
