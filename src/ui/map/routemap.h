//
// Created by Ian Parker on 25/11/2025.
//

#ifndef BLACKBOX_ROUTEMAP_H
#define BLACKBOX_ROUTEMAP_H

#include <QGeoView/QGVLayer.h>
#include <QGeoView/QGVMap.h>
#include <QGeoView/Raster/QGVIcon.h>

#include "blackbox/state.h"

class QGVLayerTilesOnline;
class QGVLayerTiles;
class BlackBoxUI;
class Route;


enum class MapMode
{
    ROUTE,
    ALL
};

class RouteMap : public QGVMap
{
    Q_OBJECT;

    BlackBoxUI* m_blackBoxUI = nullptr;

    MapMode m_mode = MapMode::ROUTE;

    QGVLayerTilesOnline* m_backgroundLayer = nullptr;
    QGVLayer* m_itemsLayer = nullptr;
    QGVLayer* m_routesLayer = nullptr;

    std::vector<Route*> m_routes;

public:
    explicit RouteMap(BlackBoxUI* blackBoxUI);
    ~RouteMap() override;

    void setMode(MapMode mode);

    void clearRoutes();

    Route* addRoute(uint64_t flightId);

    void showFlight(uint64_t flightId);

    BlackBoxUI* getBlackBoxUI() const { return m_blackBoxUI; }
    QGVLayer* getItemsLayer() const { return m_itemsLayer; }
};

#endif //BLACKBOX_ROUTEMAP_H
