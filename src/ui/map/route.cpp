//
// Created by Ian Parker on 16/12/2025.
//

#include "route.h"

#include <qabstracttextdocumentlayout.h>
#include <qtextdocument.h>
#include <QGeoView/QGVMapQGView.h>

#include <common/utils.h>

#include "routemarker.h"

using namespace std;

Route::Route(std::shared_ptr<NavigraphData> navigraphData, FlightMap* map, std::shared_ptr<Flight> flightId) :
    m_navigraphData(navigraphData),
    PolyLine(map, flightId)
{
}

vector<string> splitString(string line)
{
    vector<string> parts;

    Utils::trim(line);

    while (!line.empty())
    {
        size_t pos = line.find(' ');
        if (pos == string::npos)
        {
            pos = line.find('\t');
        }
        if (pos == string::npos)
        {
            pos = line.length();
            if (pos == 0)
            {
                break;
            }
        }
        if (pos >= 1)
        {
            string part = line.substr(0, pos);
            Utils::trim(part);
            parts.push_back(part);
        }
        if (pos == line.length())
        {
            break;
        }
        line = line.substr(pos + 1);
    }

    return parts;
}

enum class RouteState {
    START,
    ENROUTE,
    END
};

bool Route::parseRoute()
{
    m_points.clear();
    string routeStr = m_flight->route;

    Airport originAirport;
    Airport destAirport;
    auto route = splitString(routeStr);
    UFC::Coordinate lastCoord = {};

    if (!m_flight->origin.empty())
    {
        bool foundAirport = m_navigraphData->findAirport(m_flight->origin, originAirport);
        if (foundAirport)
        {
            lastCoord = originAirport.coordinate;
        }
        else
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
        if (foundAirport)
        {
            lastCoord = destAirport.coordinate;
        }
        else
        {
            printf("parseRoute: Invalid destination: %s\n", m_flight->destination.c_str());
        }
    }

    RouteState state = RouteState::START;
    vector<RoutePoint> points;
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
        if (state == RouteState::START)
        {
            if (ident.length() == 4)
            {
                bool isAirport = true;
                if (ident != m_flight->origin)
                {
                    Airport otherAirport;
                    isAirport = m_navigraphData->findAirport(ident, otherAirport);
                    if (!isAirport)
                    {
                        printf("parseRoute: Starting with different airport: %s != %s\n", ident.c_str(), m_flight->origin.c_str());
                        state = RouteState::ENROUTE;
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
            bool isNavAid = m_navigraphData->findNavAid(ident, navAid, lastCoord);
            if (isNavAid)
            {
                RoutePoint rp;
                lastCoord = navAid.coordinate;
                if (navAid.ident != navAid.name)
                {
                    rp.name = navAid.ident + " (" + navAid.name + ")";
                }
                else
                {
                    rp.name = navAid.ident;
                }
                rp.position.setLat(navAid.coordinate.latitude);
                rp.position.setLon(navAid.coordinate.longitude);
                points.push_back(rp);
            }
            else
            {
                UFC::Coordinate pos;
                isNavAid = parseLatLon(ident, pos);
                if (isNavAid)
                {
                    lastCoord = pos;
                    RoutePoint rp;
                    rp.name = ident;
                    rp.position.setLat(pos.latitude);
                    rp.position.setLon(pos.longitude);
                    points.push_back(rp);
                }
            }

            if (isNavAid)
            {
                printf("parseRoute: Waypoint: %s (%s): %ls\n", navAid.name.c_str(), ident.c_str(), lastCoord.toString().c_str());
                if ((it + 1) != route.end())
                {
                    string next = *(it + 1);

                    bool isAirway = next == "DCT";
                    if (!isAirway)
                    {
                        isAirway = m_navigraphData->findAirway(next);
                    }
                    if (isAirway)
                    {
                        printf("parseRoute: Airway: %s\n", next.c_str());
                        it++;
                    }
                }
            }
            else
            {
                bool isArrival = m_navigraphData->findArrival(m_flight->destination, ident);
                if (isArrival)
                {
                    printf("parseRoute: Arrival: %s\n", ident.c_str());
                    state = RouteState::END;
                }
                else if (ident.length() == 4)
                {
                    if (ident != m_flight->destination)
                    {
                        Airport dest;
                        bool found = m_navigraphData->findAirport(ident, dest);
                        if (found)
                        {
                            printf("parseRoute: Destination airport doesn't match!: %s != %s", dest.code.c_str(), m_flight->destination.c_str());
                            destAirport = dest;
                        }
                        else
                        {
                            printf("parseRoute: Unknown waypoint: %s\n", ident.c_str());
                        }
                    }
                }
            }
        }
    }

    printf("parseRoute: Origin: %s: %s\n", originAirport.code.c_str(), originAirport.name.c_str());
    if (originAirport.hasCoordinates)
    {
        RoutePoint rp;
        rp.name = originAirport.name;
        rp.position.setLat(originAirport.coordinate.latitude);
        rp.position.setLon(originAirport.coordinate.longitude);
        m_points.push_back(rp);
    }

    for (const auto& point : points)
    {
        m_points.push_back(point);
    }

    printf("parseRoute: Destination: %s: %s\n", destAirport.code.c_str(), destAirport.name.c_str());
    if (destAirport.hasCoordinates)
    {
        RoutePoint rp;
        rp.name = destAirport.name;
        rp.position.setLat(destAirport.coordinate.latitude);
        rp.position.setLon(destAirport.coordinate.longitude);
        m_points.push_back(rp);
    }

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

        RouteMarker* marker = new RouteMarker(point);
        point.marker = marker;

        m_map->getItemsLayer()->addItem(marker);
        m_items.push_back(marker);

        //marker->setGeometry(QGV::GeoPos(point.position.latitude(), point.position.longitude()));
        marker->bringToFront();
    }

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
#if 0
                auto name = QString::fromStdString(it->name);
                QFontMetrics metrics = painter->fontMetrics();
                QRect textRect = metrics.boundingRect(name);
                textRect.moveTo(p2.toPoint());

                auto path = QGV::createTextPath(textRect, name, font, pen.width());
                path = QGV::createTransfromScale(projAnchor(), 1).map(path);
                painter->drawPath(path);
                //painter->drawText(ptext, name);
/*
                textRect.moveTo(p2.toPoint());
                */
#endif
            }
        }
        painter->drawEllipse(p2, 50, 50);
        previous = *it;
    }
}

bool Route::parseLatLon(std::string ident, UFC::Coordinate& position)
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
        position.latitude = latDeg;
        position.longitude = lonDeg;
        return true;
    }
    else if (ident.length() == 11)
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

        printf("parseLonLat(11): %d %c %d %c\n", latDeg, latDir, lonDeg, lonDir);
    }

    return false;
}

