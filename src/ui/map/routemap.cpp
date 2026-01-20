//
// Created by Ian Parker on 25/11/2025.
//

#include "routemap.h"
#include "track.h"

#include <QGeoView/QGVLayerOSM.h>
#include <QGeoView/QGVWidgetText.h>

#include "../blackbox.h"
#include "route.h"

using namespace std;

FlightMap::FlightMap(BlackBoxUI* blackBoxUI) : m_blackBoxUI(blackBoxUI)
{
    setMouseAction(QGV::MouseAction::Tooltip, true);
    setMouseAction(QGV::MouseAction::ContextMenu, true);

    // Background layer
    auto backgroundLayer = new QGVLayerOSM();
    m_backgroundLayer = backgroundLayer;
    //backgroundLayer->setUrl("https://services.arcgisonline.com/arcgis/rest/services/World_Imagery/MapServer/tile/${z}/${y}/${x}");
    //backgroundLayer->setUrl("https://services.arcgisonline.com/arcgis/rest/services/Canvas/World_Dark_Gray_Base/MapServer/tile/${z}/${y}/${x}");
    backgroundLayer->setUrl("https://tiles.openfreemap.org/natural_earth/ne2sr/${z}/${x}/${y}.png");
    //backgroundLayer->setUrl("http://services.arcgisonline.com/ArcGIS/rest/services/Canvas/World_Dark_Gray_Base/MapServer/tile/${z}/${y}/${x}");
    //backgroundLayer->setUrl("http://basemaps.cartocdn.com/rastertiles/voyager/${z}/${x}/${y}.png");
    //backgroundLayer->setUrl("https://a.tile.opentopomap.org/${z}/${x}/${y}.png");
    //m_backgroundLayer = new PBFTileLayer();
    addItem(m_backgroundLayer);

    m_itemsLayer = new QGVLayer();
    addItem(m_itemsLayer);

    m_routesLayer = new QGVLayer();
    addItem(m_routesLayer);

    auto copyrightWidget = new QGVWidgetText();
    copyrightWidget->setText("<small>© OpenStreetMap contributors</small>");
    copyrightWidget->setAnchor(QPoint(5, 5), { Qt::RightEdge, Qt::BottomEdge });
    copyrightWidget->setAutoFillBackground(true);
    addWidget(copyrightWidget);

    m_screenshotIcon = new QImage(":/images/camera.svg");
}

FlightMap::~FlightMap()
{
}

void FlightMap::setMode(MapMode mode)
{
    if (m_mode == mode)
    {
        return;
    }

    m_mode = mode;
    clearRoutes();
    if (m_mode == MapMode::ALL)
    {
        for (auto flight : m_blackBoxUI->getFlights())
        {
            addRoute(flight);
        }
    }
}

void FlightMap::clearRoutes()
{
    for (auto track : m_tracks)
    {
        track->removeFromMap();
        m_routesLayer->removeItem(track);
        delete track;
    }
    m_tracks.clear();

    for (auto route : m_routes)
    {
        route->removeFromMap();
        m_routesLayer->removeItem(route);
        delete route;
    }
    m_routes.clear();
}

Track* FlightMap::addRoute(shared_ptr<Flight> flight)
{
    Track* route = new Track(this, flight);
    route->updateRoute();
    m_routesLayer->addItem(route);
    m_tracks.push_back(route);
    return route;
}

void FlightMap::showFlight(uint64_t flightId)
{
    for (auto route : m_tracks)
    {
        if (route->getFlight()->id == flightId)
        {
            route->showRoute();
            return;
        }
    }

    refreshRoutes(flightId);
}

void FlightMap::refreshRoutes(uint64_t flightId)
{
    if (m_mode == MapMode::ROUTE)
    {
        clearRoutes();

        auto flight = m_blackBoxUI->getFlight(flightId);
        if (flight != nullptr)
        {
            Route* route = new Route(m_blackBoxUI->getNavigraph(), this, flight);
            if (route->createRoute())
            {
                m_routesLayer->addItem(route);
                m_routes.push_back(route);
            }

            Track* track = addRoute(flight);
            track->showRoute();
        }
    }
}

