//
// Created by Ian Parker on 25/11/2025.
//

#include "routemap.h"
#include "route.h"

#include <QTimer>

#include <QGeoView/QGVLayerOSM.h>
#include <QGeoView/QGVWidgetText.h>

#include "../blackbox.h"
#include "landingicon.h"

using namespace std;

RouteMap::RouteMap(BlackBoxUI* blackBoxUI) : m_blackBoxUI(blackBoxUI)
{
    setMouseAction(QGV::MouseAction::Tooltip, true);
    setMouseAction(QGV::MouseAction::ContextMenu, true);

    // Background layer
    auto backgroundLayer = new QGVLayerOSM();
    m_backgroundLayer = backgroundLayer;
    //backgroundLayer->setUrl("https://services.arcgisonline.com/arcgis/rest/services/World_Imagery/MapServer/tile/${z}/${y}/${x}");
    //backgroundLayer->setUrl("https://services.arcgisonline.com/arcgis/rest/services/Canvas/World_Dark_Gray_Base/MapServer/tile/${z}/${y}/${x}");
    //backgroundLayer->setUrl("http://services.arcgisonline.com/ArcGIS/rest/services/Canvas/World_Dark_Gray_Base/MapServer/tile/${z}/${y}/${x}");
    //backgroundLayer->setUrl("http://basemaps.cartocdn.com/rastertiles/voyager/${z}/${x}/${y}.png");
    //backgroundLayer->setUrl("https://a.tile.opentopomap.org/${z}/${x}/${y}.png");
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
}

RouteMap::~RouteMap()
{
}

void RouteMap::setMode(MapMode mode)
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
            Route* route = addRoute(flight.first);
        }
    }

}

void RouteMap::clearRoutes()
{
    for (auto route : m_routes)
    {
        route->removeFromMap();
        m_routesLayer->removeItem(route);
        delete route;
    }
    m_routes.clear();
}

Route* RouteMap::addRoute(uint64_t flightId)
{
    Route* route = new Route(this, flightId);
    route->updateRoute();
    m_routesLayer->addItem(route);
    m_routes.push_back(route);
    return route;
}

void RouteMap::showFlight(uint64_t flightId)
{
    for (auto route : m_routes)
    {
        if (route->getFlight() == flightId)
        {
            route->showRoute();
            return;
        }
    }

    if (m_mode == MapMode::ROUTE)
    {
        clearRoutes();

        auto it = m_blackBoxUI->getFlights().find(flightId);
        if (it != m_blackBoxUI->getFlights().end())
        {
            Route* route = addRoute(flightId);
            route->showRoute();
        }
    }
}
