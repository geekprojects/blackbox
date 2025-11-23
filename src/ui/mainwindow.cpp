//
// Created by Ian Parker on 20/11/2025.
//

#include "mainwindow.h"

#include <QDir>
#include <QNetworkDiskCache>
#include <QTimer>
#include <QGeoView/QGVWidgetScale.h>
#include <QGeoView/Raster/QGVIcon.h>
#include <QGeoView/QGVLayerOSM.h>

using namespace std;

void setupCachedNetworkAccessManager(QObject* parent)
{
    QDir("cacheDir");//.removeRecursively();
    auto cache = new QNetworkDiskCache(parent);
    cache->setCacheDirectory("cacheDir");
    auto manager = new QNetworkAccessManager(parent);
    manager->setCache(cache);
    QGV::setNetworkManager(manager);
}

MainWindow::MainWindow()
{
    setWindowTitle("BlackBox Flight Tracker");

    mMap = new QGVMap(this);
    mMap->addWidget(new QGVWidgetScale(Qt::Horizontal));
    mMap->setMouseAction(QGV::MouseAction::All, true);
    setCentralWidget(mMap);

    setupCachedNetworkAccessManager(this);

    m_dataStore.init();

    // Background layer
    //auto osmLayer = new QGVLayerGoogle();
    //osmLayer->setType(QGV::TilesType::Hybrid);
    auto osmLayer = new QGVLayerOSM();
    //osmLayer->setUrl("https://services.arcgisonline.com/arcgis/rest/services/World_Imagery/MapServer/tile/${z}/${y}/${x}");
    //osmLayer->setUrl("https://services.arcgisonline.com/arcgis/rest/services/Canvas/World_Dark_Gray_Base/MapServer/tile/${z}/${y}/${x}");
    //osmLayer->setUrl("http://services.arcgisonline.com/ArcGIS/rest/services/Canvas/World_Dark_Gray_Base/MapServer/tile/${z}/${y}/${x}");
    //osmLayer->setUrl("http://basemaps.cartocdn.com/dark_all/${z}/${x}/${y}.png");
    //osmLayer->setUrl("https://a.tile.opentopomap.org/${z}/${x}/${y}.png");
    mMap->addItem(osmLayer);

    mItemsLayer = new QGVLayer();
    mMap->addItem(mItemsLayer);

    auto flights = m_dataStore.fetchFlights();

    //for (auto flight : flights)
    {
        m_route = new Route(Qt::blue);
        updateRoute();

        mItemsLayer->addItem(m_route);
    }


    /*
    const QGV::GeoRect itemTargetArea = mMap->getProjection()->projToGeo(mMap->getCamera().projRect());

    m_line = new Line(QGV::GeoPos(51.187814, 0.266895), QGV::GeoPos(51, 0), Qt::red);
    m_line->setFlag(QGV::ItemFlag::Clickable, true);
    m_line->setFlag(QGV::ItemFlag::Movable, true);
    mItemsLayer->addItem(m_line);
    */

    QTimer::singleShot(100, this, [this]()
    {
        auto target = m_route->getRect();
        mMap->cameraTo(QGVCameraActions(mMap).scaleTo(target));
    });

    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::updateRoute);
    timer->start(1000);
}

MainWindow::~MainWindow()
{
}

void MainWindow::updateRoute()
{
    if (true)
    {
        return;
    }
    auto stateUpdates = m_dataStore.fetchUpdates(3, m_lastTimestamp);
    vector<Point> points;
    for (const State& state : stateUpdates)
    {
        Point p;
        p.position = QGV::GeoPos(state.position.latitude, state.position.longitude);
        p.altitude = state.position.altitude;
        points.push_back(p);
        m_lastTimestamp = state.timestamp;

        if (state.flightPhase == FlightPhase::LANDING && m_lastState.flightPhase != FlightPhase::LANDING)
        {
            QImage planeIcon("../data/images/landing.png");
            auto* item = new QGVIcon();
            item->loadImage(planeIcon);
            item->setGeometry(QGV::GeoPos(p.position.latitude(), p.position.longitude()), QSizeF(40, 40));
            mItemsLayer->addItem(item);
            item->bringToFront();
        }
        if (state.flightPhase == FlightPhase::TAKE_OFF && m_lastState.flightPhase != FlightPhase::TAKE_OFF)
        {
            QImage planeIcon("../data/images/airport.png");
            auto* item = new QGVIcon();
            item->loadImage(planeIcon);
            item->setGeometry(QGV::GeoPos(p.position.latitude(), p.position.longitude()), QSizeF(40, 40));
            mItemsLayer->addItem(item);
            item->bringToFront();
        }

        m_lastState = state;
    }

    if (!points.empty())
    {
        m_route->addPoints(points);
    }
}
