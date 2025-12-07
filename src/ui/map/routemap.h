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

enum class LineType
{
    ALTITUDE,
    SPEED,
    G_FORCE
};

class RouteMap : public QGVMap
{
    Q_OBJECT;

    BlackBoxUI* m_blackBoxUI = nullptr;

    MapMode m_mode = MapMode::ROUTE;
    LineType m_lineType = LineType::ALTITUDE;

    QGVLayerTilesOnline* m_backgroundLayer = nullptr;
    QGVLayer* m_itemsLayer = nullptr;
    QGVLayer* m_routesLayer = nullptr;

    std::vector<Route*> m_routes;

    QImage* m_screenshotIcon;

public:
    explicit RouteMap(BlackBoxUI* blackBoxUI);
    ~RouteMap() override;

    void setMode(MapMode mode);

    void clearRoutes();

    Route* addRoute(uint64_t flightId);

    void showFlight(uint64_t flightId);

    MapMode getMode() const { return m_mode; }
    LineType getLineType() const { return m_lineType; }

    BlackBoxUI* getBlackBoxUI() const { return m_blackBoxUI; }
    QGVLayer* getItemsLayer() const { return m_itemsLayer; }

    QImage* getScreenshotIcon() const { return m_screenshotIcon; }
};

#endif //BLACKBOX_ROUTEMAP_H
