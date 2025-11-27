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

class RouteMap : public QGVMap
{
    Q_OBJECT;

    BlackBoxUI* m_blackBoxUI = nullptr;

    QGVLayerTilesOnline* m_backgroundLayer = nullptr;
    QGVLayer* m_itemsLayer = nullptr;

    Route* m_route = nullptr;
    State m_lastState;
    uint64_t m_lastTimestamp = 0;

    std::vector<QGVItem*> m_items;

    QImage* m_planeIcon = nullptr;
    QGVIcon* m_positionIcon = nullptr;

public:
    explicit RouteMap(BlackBoxUI* blackBoxUI);
    ~RouteMap() override;

    void resetRoute();
    void updateRoute();


};

#endif //BLACKBOX_ROUTEMAP_H
