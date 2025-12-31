//
// Created by Ian Parker on 26/12/2025.
//

#include "routeparser.h"

#include "route.h"
#include "common/utils.h"

using namespace std;

enum class RouteState
{
    START,
    ENROUTE,
    END
};

bool RouteParser::parseRoute(
    const std::shared_ptr<Flight>& flight,
    Airport& originAirport,
    Airport& destAirport,
    UFC::Coordinate& lastCoord,
    std::vector<RoutePoint>& resolvedPoints)
{
    RouteState state = RouteState::START;
    vector<std::shared_ptr<RoutePoint>> parsedPoints;
    auto route = Utils::splitString(flight->route);
    shared_ptr<RoutePoint> previousPoint = nullptr;
    for (auto it = route.begin(); it != route.end(); ++it)
    {
        string part = *it;
        string ident = part;
        string info;
        auto idx = ident.find('/');
        if (idx != string::npos)
        {
            ident = part.substr(0, idx);
            info = part.substr(idx + 1);
        }

        printf("parseRoute: ident=%s, info=%s\n", ident.c_str(), info.c_str());
        shared_ptr<RoutePoint> rp = nullptr;
        if (state == RouteState::START)
        {
            if (ident.length() == 4)
            {
                bool isAirport = true;
                if (ident != flight->origin)
                {
                    Airport otherAirport;
                    isAirport = m_navigraphData->findAirport(ident, otherAirport);
                    if (!isAirport)
                    {
                        printf("parseRoute: Starting with different airport: %s != %s\n", ident.c_str(), flight->origin.c_str());
                    }
                    else
                    {
                        originAirport = otherAirport;
                    }
                }

                if (isAirport)
                {
                    printf("parseRoute: Origin: %s (%s) Runway: %s\n", originAirport.code.c_str(), ident.c_str(), info.c_str());
                }
                else
                {
                    printf("parseRoute: Invalid airport: %s\n", ident.c_str());
                    state = RouteState::ENROUTE;
                    it--;
                }
            }
            else
            {
                bool isDeparture = m_navigraphData->findDeparture(originAirport.code, ident);
                if (isDeparture)
                {
                    printf("parseRoute: Departure: %s\n", ident.c_str());
                }
                else
                {
                    // Not a departure, probably part of the route
                    state = RouteState::ENROUTE;
                }
            }
        }
        if (state == RouteState::ENROUTE)
        {
            NavAid navAid;
            rp = parseWaypoint(lastCoord, ident, navAid);

            if (rp != nullptr)
            {
                printf("parseRoute: Waypoint: %s (%s): %ls\n", navAid.name.c_str(), ident.c_str(), lastCoord.toString().c_str());
                parsedPoints.push_back(rp);
                parseAirway(route, it, rp);
            }
            else
            {
                if (m_navigraphData->findArrival(flight->destination, ident))
                {
                    printf("parseRoute: Arrival: %s\n", ident.c_str());
                    state = RouteState::END;
                }
                else if (ident.length() == 4)
                {
                    if (ident != flight->destination)
                    {
                        Airport dest;
                        bool found = m_navigraphData->findAirport(ident, dest);
                        if (found)
                        {
                            printf("parseRoute: Destination airport doesn't match!: %s != %s", dest.code.c_str(), flight->destination.c_str());
                            destAirport = dest;
                            state = RouteState::END;
                        }
                        else
                        {
                            printf("parseRoute: Unknown waypoint: %s\n", ident.c_str());
                        }
                    }
                    else
                    {
                        state = RouteState::END;
                    }
                }
            }
        }
    }

    printf("parseRoute: Origin: %s: %s\n", originAirport.code.c_str(), originAirport.name.c_str());
    addAirport(originAirport, resolvedPoints);

    expandAirways(resolvedPoints, parsedPoints);
    addAirport(destAirport, resolvedPoints);

    printf("parseRoute: Destination: %s: %s\n", destAirport.code.c_str(), destAirport.name.c_str());

    return true;
}

void RouteParser::expandAirways(std::vector<RoutePoint> &resolvedPoints, vector<std::shared_ptr<RoutePoint>> parsedPoints)
{
    for (auto it = parsedPoints.begin(); it != parsedPoints.end(); ++it)
    {
        const auto& point = *it;
        resolvedPoints.push_back(*point);
        if ((it + 1) != parsedPoints.end())
        {
            auto nextPoint = *(it + 1);
            printf("parseRoute: %s -> %s\n", point->name.c_str(), nextPoint->name.c_str());
            if (point->type == NavAidType::WAYPOINT &&
                point->sourceId != 0 &&
                nextPoint->type == NavAidType::WAYPOINT &&
                nextPoint->sourceId != 0)
            {
                if (!point->airway.empty())
                {
                    printf("parseRoute:  -> Airway: %s\n", point->airway.c_str());
                    vector<NavAid> airwayPoints;
                    m_navigraphData->expandAirway(point->airway, point->sourceId, nextPoint->sourceId, airwayPoints);
                    for (auto const& ap : airwayPoints)
                    {
                        RoutePoint rp;
                        rp.name = ap.ident;
                        rp.position.setLat(ap.coordinate.latitude);
                        rp.position.setLon(ap.coordinate.longitude);
                        rp.type = ap.type;
                        resolvedPoints.push_back(rp);
                    }
                }
                else
                {
                    printf("parseRoute:  -> No airway connecting to next point\n");
                }
            }
        }
    }
}


vector<RoutePoint> RouteParser::generateGreatCirclePaths(std::vector<RoutePoint> &points)
{
    vector<RoutePoint> greatCircleRoute;
    for (auto it = points.begin(); it != points.end(); ++it)
    {
        RoutePoint const& rp = *it;
        bool added = false;
        if (it != points.begin())
        {
            auto const& prev = *(it - 1);
            auto distance = Utils::distance(prev.position.latitude(), prev.position.longitude(), rp.position.latitude(), rp.position.longitude());
            printf("parseRoute: Distance from %s to %s: %0.2f\n", prev.name.c_str(), rp.name.c_str(), distance);
            if (distance > 300.0f)
            {
                generateGreatCirclePath(greatCircleRoute, rp, prev, distance);
                added = true;
            }
        }
        if (!added)
        {
            greatCircleRoute.push_back(rp);
        }
    }
    return greatCircleRoute;
}

void RouteParser::generateGreatCirclePath(
    vector<RoutePoint>& greatCircleRoute,
    RoutePoint const &rp,
    RoutePoint const &prev,
    double distance)
{
    GreatCircle gc(prev.position, rp.position);
    auto arcPoints = gc.arc(static_cast<int>(distance / 100.0f));
    for (auto arcIt = arcPoints.begin(); arcIt != arcPoints.end(); ++arcIt)
    {
        auto const& p = *arcIt;
        RoutePoint gcPoint;
        if ((arcIt + 1) == arcPoints.end())
        {
            gcPoint.name = rp.name;
        }
        gcPoint.position.setLat(p.latitude());
        gcPoint.position.setLon(p.longitude());
        greatCircleRoute.push_back(gcPoint);
    }
}

void RouteParser::addAirport(const Airport& originAirport, std::vector<RoutePoint> &resolvedPoints)
{
    if (originAirport.hasCoordinates)
    {
        RoutePoint rp;
        rp.name = originAirport.name;
        rp.position.setLat(originAirport.coordinate.latitude);
        rp.position.setLon(originAirport.coordinate.longitude);
        resolvedPoints.push_back(rp);
    }
}

std::shared_ptr<RoutePoint> RouteParser::parseWaypoint(UFC::Coordinate& lastCoord, const string& ident, NavAid& navAid) const
{
    shared_ptr<RoutePoint> rp = nullptr;

    bool isNavAid = m_navigraphData->findNavAid(ident, navAid, lastCoord);
    if (isNavAid)
    {
        rp = make_shared<RoutePoint>();
        lastCoord = navAid.coordinate;
        if (navAid.ident != navAid.name)
        {
            rp->name = navAid.ident + " (" + navAid.name + ")";
        }
        else
        {
            rp->name = navAid.ident;
        }
        rp->position.setLat(navAid.coordinate.latitude);
        rp->position.setLon(navAid.coordinate.longitude);
        rp->type = navAid.type;
        rp->sourceId = navAid.sourceId;
    }
    else
    {
        UFC::Coordinate pos;
        isNavAid = parseLatLon(ident, pos);
        if (isNavAid)
        {
            lastCoord = pos;
            rp = make_shared<RoutePoint>();
            rp->name = ident;
            rp->position.setLat(pos.latitude);
            rp->position.setLon(pos.longitude);
            rp->type = NavAidType::WAYPOINT;
        }
    }
    return rp;
}

void RouteParser::parseAirway(std::vector<std::string> route, std::vector<std::string>::iterator &it, const shared_ptr<RoutePoint> &rp)
{
    if ((it + 1) != route.end())
    {
        string next = *(it + 1);

        bool isAirway = false;
        if (next == "DCT")
        {
            isAirway = true;
            rp->airway = "DCT";
        }
        if (!isAirway)
        {
            isAirway = m_navigraphData->findAirway(next);
        }
        if (isAirway)
        {
            printf("parseRoute: Airway: %s\n", next.c_str());
            rp->airway = next;
            it++;
        }
    }
}


RouteParser::RouteParser(const std::shared_ptr<NavigraphData> &navigraphData) :
    Logger("RouteParser"),
    m_navigraphData(navigraphData)
{
}

bool RouteParser::createRoute(const std::shared_ptr<Flight>& flight, std::vector<RoutePoint>& points)
{
    string routeStr = flight->route;

    Airport originAirport;
    Airport destAirport;
    UFC::Coordinate lastCoord = {};

    if (!flight->origin.empty())
    {
        bool foundAirport = m_navigraphData->findAirport(flight->origin, originAirport);
        if (foundAirport)
        {
            lastCoord = originAirport.coordinate;
        }
        else
        {
            printf("parseRoute: Invalid origin: %s\n", flight->origin.c_str());
        }
    }
    else
    {
        // No origin set??
        printf("parseRoute: No origin set! This may end badly\n");
        originAirport.code = flight->origin;
    }

    if (!flight->destination.empty())
    {
        bool foundAirport = m_navigraphData->findAirport(flight->destination, destAirport);
        if (foundAirport)
        {
            lastCoord = destAirport.coordinate;
        }
        else
        {
            printf("parseRoute: Invalid destination: %s\n", flight->destination.c_str());
        }
    }

    parseRoute(flight, originAirport, destAirport, lastCoord, points);

    vector<RoutePoint> greatCircleRoute = generateGreatCirclePaths(points);
    points = greatCircleRoute;

    return true;
}


bool RouteParser::parseLatLon(const std::string& ident, UFC::Coordinate& position)
{
    if (ident.length() == 7)
    {
        // 51N025E
        int latDeg;
        int lonDeg;
        char latDir;
        char lonDir;

        sscanf(ident.c_str(), "%d%c%d%c", &latDeg, &latDir, &lonDeg, &lonDir);

        if ((latDir != 'N' && latDir != 'S') || (lonDir != 'E' && lonDir != 'W'))
        {
            return false;
        }

        if (latDir == 'S')
        {
            latDeg = -latDeg;
        }
        if (lonDir == 'W')
        {
            lonDeg = -lonDeg;
        }

        printf("parseLonLat(7): %d %c %d %c\n", latDeg, latDir, lonDeg, lonDir);
        position.latitude = static_cast<float>(latDeg);
        position.longitude = static_cast<float>(lonDeg);
        return true;
    }

    if (ident.length() == 11)
    {
        //5220N03305E
        int latDeg;
        int lonDeg;
        char latDir;
        char lonDir;
        sscanf(ident.c_str(), "%d%c%d%c", &latDeg, &latDir, &lonDeg, &lonDir);
        if ((latDir != 'N' && latDir != 'S') || (lonDir != 'E' && lonDir != 'W'))
        {
            return false;
        }

        const int latMins = latDeg % 100;
        const int lonMins = lonDeg % 100;

        latDeg -= latMins;
        lonDeg -= lonMins;
        latDeg /= 100;
        lonDeg /= 100;

        if (latDir == 'S')
        {
            latDeg = -latDeg;
        }
        if (lonDir == 'W')
        {
            lonDeg = -lonDeg;
        }

        const float lat = (float)latDeg + ((float)latMins / 60.0f);
        const float lon = (float)lonDeg + ((float)lonMins / 60.0f);

        printf("parseLonLat(11): %d %c %d %c -> %f, %f\n", latDeg, latDir, lonDeg, lonDir, lat, lon);
        position.latitude = lat;
        position.longitude = lon;
        return true;
    }

    return false;
}
