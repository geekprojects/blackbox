//
// Created by Ian Parker on 25/11/2025.
//

#include "routemap.h"
#include "route.h"

#include <QTimer>
#include <QLabel>

#include <QGeoView/QGVLayerOSM.h>

#include "blackbox.h"
#include "landingicon.h"

using namespace std;

RouteMap::RouteMap(BlackBoxUI* blackBoxUI) : m_blackBoxUI(blackBoxUI)
{
    setMouseAction(QGV::MouseAction::Tooltip, true);
    setMouseAction(QGV::MouseAction::ContextMenu, true);

    // Background layer
    //auto osmLayer = new QGVLayerGoogle();
    //osmLayer->setType(QGV::TilesType::Hybrid);
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

    m_planeIcon = new QImage("../data/images/plane-red.png");

    m_positionIcon = new QGVIcon();
    m_positionIcon->loadImage(*m_planeIcon);
    m_positionIcon->setVisible(false);
    m_itemsLayer->addItem(m_positionIcon);
    m_route = new Route(Qt::blue);
    m_itemsLayer->addItem(m_route);

    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &RouteMap::updateRoute);
    timer->start(1000);

}

RouteMap::~RouteMap()
{
}

void RouteMap::resetRoute()
{
    m_route->clear();
    m_lastTimestamp = 0;

    for (auto item : m_items)
    {
        m_itemsLayer->removeItem(item);
    }
    m_items.clear();

    updateRoute();
    refreshMap();

    QTimer::singleShot(100, this, [this]()
    {
        auto target = m_route->getRect();
        double top = target.latTop();
        double bottom = target.latBottom();
        double left = target.lonLeft();
        double right = target.lonRight();

        double width = abs(right - left);
        double height = abs(bottom - top);

        // Add some margin around the route
        double vertMargin = width * 0.1;
        double horizMargin = height * 0.1;

        if (top > 0.0)
        {
            top += vertMargin;
        }
        else
        {
            top -= vertMargin;
        }
        if (bottom > 0.0)
        {
            bottom -= vertMargin;
        }
        else
        {
            bottom += vertMargin;
        }

        if (left > 0.0)
        {
            left += horizMargin;
        }
        else
        {
            left -= horizMargin;
        }
        if (right > 0.0)
        {
            right -= horizMargin;
        }
        else
        {
            right += horizMargin;
        }

        QGV::GeoRect viewRect(top, left, bottom, right);

        flyTo(QGVCameraActions(this).scaleTo(viewRect));
    });
}

void RouteMap::updateRoute()
{
    auto stateUpdates = m_blackBoxUI->getDataStore().fetchUpdates(m_blackBoxUI->getCurrentFlight().id, m_lastTimestamp);
    if (stateUpdates.empty())
    {
        return;
    }

    vector<Point> points;
    for (const State& state : stateUpdates)
    {
        Point p;
        p.position = QGV::GeoPos(state.position.latitude, state.position.longitude);
        //p.altitude = state.position.altitude;
        p.altitude = state.agl;
        if (p.altitude < 0.0f)
        {
            p.altitude = 0.0f;
        }
        p.heading = state.yaw;
        points.push_back(p);
        m_lastTimestamp = state.timestamp;

        if (state.flightPhase == FlightPhase::LANDING && m_lastState.flightPhase != FlightPhase::LANDING)
        {
            auto* item = new LandingIcon(state);
            item->setGeometry(QGV::GeoPos(p.position.latitude(), p.position.longitude()), QSizeF(40, 40));
            m_items.push_back(item);
            m_itemsLayer->addItem(item);
            item->bringToFront();
        }
        if (state.flightPhase == FlightPhase::TAKE_OFF && m_lastState.flightPhase != FlightPhase::TAKE_OFF)
        {
            QImage planeIcon("../data/images/airport.png");
            auto* item = new QGVIcon();
            item->loadImage(planeIcon);
            item->setGeometry(QGV::GeoPos(p.position.latitude(), p.position.longitude()), QSizeF(40, 40));
            m_items.push_back(item);
            m_itemsLayer->addItem(item);
            item->bringToFront();
        }

        m_lastState = state;
    }

    m_blackBoxUI->setState(m_lastState);

    if (!points.empty())
    {
        m_route->addPoints(points);

        Point point = m_route->getLastPosition();

        QTransform transform;
        transform.rotate(point.heading);

        QImage image = m_planeIcon->transformed(transform);
        m_positionIcon->loadImage(image);

        m_positionIcon->setGeometry(
            QGV::GeoPos(point.position.latitude(), point.position.longitude()),
            QSizeF(40, 40));
        m_positionIcon->setVisible(true);
        m_positionIcon->bringToFront();
    }
}
