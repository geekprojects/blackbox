//
// Created by Ian Parker on 14/11/2025.
//

#include "route.h"

#include <filesystem>
#include <cfloat>

#include <QTimer>

#include <QGeoView/QGVCamera.h>
#include <QGeoView/QGVMapQGView.h>

#include "landingicon.h"
#include "screenshoticon.h"
#include "../blackbox.h"

using namespace std;

QColor interpolate(QColor start,QColor end,double ratio)
{
    return QColor::fromRgb(
        static_cast<int>(ratio * start.red() + (1 - ratio) * end.red()),
        static_cast<int>(ratio * start.green() + (1 - ratio) * end.green()),
        static_cast<int>(ratio * start.blue() + (1 - ratio) * end.blue()));
}

Route::Route(RouteMap* map, uint64_t flightId) : m_map(map), m_flightId(flightId)
{
    setFlag(QGV::ItemFlag::Clickable);

    m_planeIcon = new QImage(":/images/plane-red.png");

    m_positionIcon = new QGVIcon();
    m_positionIcon->loadImage(*m_planeIcon);
    m_positionIcon->setVisible(false);
    m_map->getItemsLayer()->addItem(m_positionIcon);
    m_items.push_back(m_positionIcon);

    m_updateTimer = new QTimer(this);
    connect(m_updateTimer, &QTimer::timeout, this, &Route::updateRoute);
    m_updateTimer->start(1000);
}

void Route::addPoints(std::vector<Point> points)
{
    m_points.insert(m_points.end(), points.begin(), points.end());
    printf("addPoints: Added %ld points, we now have %ld\n", points.size(), m_points.size());

    m_maxAltitude = 0.001f;
    if (!m_points.empty())
    {
        m_boundingRect = QGV::GeoRect(m_points[0].position, m_points[0].position);

        double minLat = m_points[0].position.latitude();
        double maxLat = m_points[0].position.latitude();
        double minLon = m_points[0].position.longitude();
        double maxLon = m_points[0].position.longitude();

        for (const auto& point : m_points)
        {
            if (point.position.latitude() < minLat)
            {
                minLat = point.position.latitude();
            }
            if (point.position.latitude() > maxLat)
            {
                maxLat = point.position.latitude();
            }
            if (point.position.longitude() < minLon)
            {
                minLon = point.position.longitude();
            }
            if (point.position.longitude() > maxLon)
            {
                maxLon = point.position.longitude();
            }
            if (point.altitude > m_maxAltitude)
            {
                m_maxAltitude = point.altitude;
            }
        }
        m_boundingRect = QGV::GeoRect(
            QGV::GeoPos(minLat, minLon),
            QGV::GeoPos(maxLat, maxLon));
    }
    printf("Max Altitude: %0.2f\n", m_maxAltitude);

    // Geo coordinates need to be converted manually again to projection
    if (getMap() != nullptr)
    {
        onProjection(getMap());

        // Now we can inform QGV about changes for this
        resetBoundary();
        refresh();
    }
}

void Route::clear()
{
    m_points.clear();
    m_boundingRect = QGV::GeoRect();
    refresh();
}

QGV::GeoRect Route::getRect() const
{
    return m_boundingRect;
}

Point Route::getLastPosition()
{
    if (!m_points.empty())
    {
        return m_points.back();
    }
    return {};
}

void Route::onProjection(QGVMap* geoMap)
{
    QGVDrawItem::onProjection(geoMap);
    for (auto& point : m_points)
    {
        point.projected = geoMap->getProjection()->geoToProj(point.position);
    }

    m_boundingRectProjected = QRectF(
        geoMap->getProjection()->geoToProj(m_boundingRect.topLeft()),
        geoMap->getProjection()->geoToProj(m_boundingRect.bottomRight()));
}

QPainterPath Route::projShape() const
{
    QPainterPath path;

    if (!m_points.empty())
    {
        path.moveTo(m_points.front().projected);
        for (auto it = m_points.begin(); it != m_points.end(); ++it)
        {
            if (it != m_points.begin())
            {
                path.lineTo(it->projected);
            }
        }
    }

    return path;
}


void Route::projPaint(QPainter* painter)
{
    if (m_points.empty())
    {
        // Nothing to show
        return;
    }

    QPen pen;
    pen.setWidth(10);
    pen.setCosmetic(true);

    auto rect = m_map->geoView()->getCamera().projRect();

    QColor colour1;
    QColor colour2;
    if (m_map->getLineType() == LineType::G_FORCE)
    {
        colour1 =  QColor(0, 255, 0);
        colour2 =  QColor(255, 0, 0);
    }
    else
    {
        colour1 =  QColor(0, 255, 0);
        colour2 =  QColor(82, 78, 221);
    }

    Point previous;
    for (auto it = m_points.begin(); it != m_points.end(); ++it)
    {
        if (it != m_points.begin())
        {
            QPointF const& p1 = previous.projected;
            QPointF const& p2 = it->projected;
            if (rect.contains(p1) || rect.contains(p2))
            {
                float altitude = (it->altitude + previous.altitude) / 2.0f;
                pen.setColor(interpolate(colour2, colour1, altitude / m_maxAltitude));
                painter->setPen(pen);
                painter->drawLine(p1, p2);
            }
        }
        previous = *it;
    }
}

QPointF Route::projAnchor() const
{
    return m_boundingRectProjected.center();
}

void nearestpointonline2D(const QGV::GeoPos& a, const  QGV::GeoPos& b, const QGV::GeoPos& point, double& lineQx, double& lineQy)
{
    double PxminusAx = point.latitude() - a.latitude();
    double PyminusAy = point.longitude() - a.longitude();
    double BxminusAx = b.latitude() - a.latitude();
    double ByminusAy = b.longitude() - a.longitude();
    double lenABsqrd = BxminusAx*BxminusAx + ByminusAy*ByminusAy;
    // Q = A + lambda*(B - A)
    double lambda = (PxminusAx*BxminusAx + PyminusAy*ByminusAy) / lenABsqrd;
    if (lambda < 0.0f) lambda = 0.0f;
    if (lambda > 1.0f) lambda = 1.0f;
    lineQx = a.latitude() + lambda*BxminusAx;
    lineQy = a.longitude() + lambda*ByminusAy;
}

double pointdistfromline2D(const QGV::GeoPos& a, const QGV::GeoPos& b, const QGV::GeoPos& point)
{
    // Returns minimum distance from P to straight line between A and B.
    double pointQx;
    double pointQy;
    nearestpointonline2D(a, b, point, pointQx, pointQy);
    double PxminusQx = point.latitude() - pointQx;
    double PyminusQy = point.longitude() - pointQy;
    return sqrt(PxminusQx*PxminusQx + PyminusQy*PyminusQy);
}

QString Route::projTooltip(const QPointF& projPos) const
{
    auto geo = getMap()->getProjection()->projToGeo(projPos);

    QGV::GeoPos previous;
    Point closest;
    double closestDiff = DBL_MAX;
    bool found = false;
    for (auto& point : m_points)
    {
        double d = pointdistfromline2D(previous, point.position, geo);
        if (d < 0.1 && d < closestDiff)
        {
            closestDiff = d;
            closest = point;
            found = true;
        }
        previous = point.position;
    }

    if (found)
    {
        char buf[1024];
        snprintf(buf, 1024, "Distance: %.2f, altitude: %0.2f", closestDiff, closest.altitude);
        return buf;
    }
    return "";
}

void Route::updateRoute()
{
    BlackBoxUI* ui = m_map->getBlackBoxUI();
    auto stateUpdates = ui->getDataStore().fetchUpdates(m_flightId, m_lastTimestamp);
    if (stateUpdates.empty())
    {
        return;
    }

    vector<Point> points;
    for (const State& state : stateUpdates)
    {
        Point p;
        p.position = QGV::GeoPos(state.position.latitude, state.position.longitude);

        switch (m_map->getLineType())
        {
            case LineType::ALTITUDE:
                p.altitude = state.position.altitude;
                break;
            case LineType::SPEED:
                p.altitude = state.groundSpeed;
                break;
            case LineType::G_FORCE:
                p.altitude = fabs(state.gForce - 0.98f);
                break;
        }

        if (p.altitude < 0.0f)
        {
            p.altitude = 0.0f;
        }
        p.heading = state.yaw;
        points.push_back(p);
        m_lastTimestamp = state.timestamp;

        if (m_map->getMode() == MapMode::ROUTE)
        {
            if (state.flightPhase == FlightPhase::LANDING && m_lastState.flightPhase != FlightPhase::LANDING)
            {
                auto* item = new LandingIcon(state);
                item->setGeometry(QGV::GeoPos(p.position.latitude(), p.position.longitude()), QSizeF(20, 20));
                m_items.push_back(item);
                m_map->getItemsLayer()->addItem(item);
                item->bringToFront();
            }
            if (state.flightPhase == FlightPhase::TAKE_OFF && m_lastState.flightPhase != FlightPhase::TAKE_OFF)
            {
                QImage planeIcon(":/images/airport.png");
                auto* item = new QGVIcon();
                item->loadImage(planeIcon);
                item->setGeometry(QGV::GeoPos(p.position.latitude(), p.position.longitude()), QSizeF(20, 20));
                m_items.push_back(item);
                m_map->getItemsLayer()->addItem(item);
                item->bringToFront();
            }
        }

        m_lastState = state;
    }

    ui->setState(m_lastState);

    if (!points.empty())
    {
        addPoints(points);

        if (m_map->getMode() == MapMode::ROUTE)
        {
            Point point = getLastPosition();

            QTransform transform;
            transform.rotate(point.heading);

            QImage image = m_planeIcon->transformed(transform);
            m_positionIcon->loadImage(image);

            m_positionIcon->setGeometry(
                QGV::GeoPos(point.position.latitude(), point.position.longitude()),
                QSizeF(40, 40));
            m_positionIcon->setVisible(true);

            updateScreenshots();

            m_positionIcon->bringToFront();
        }
        else
        {
            m_positionIcon->setVisible(false);
        }
    }
}

void Route::updateScreenshots()
{
    BlackBoxUI* ui = m_map->getBlackBoxUI();

    auto screenshots = ui->getDataStore().fetchScreenshots(m_flightId, m_lastScreenshotTimestamp);
    for (auto const& screenshot : screenshots)
    {
        auto icon = new ScreenshotIcon(screenshot.path, screenshot.position);
        connect(m_map, &QGVMap::scaleChanged, icon, &ScreenshotIcon::scaleChanged);

        m_items.push_back(icon);
        m_screenshots.push_back(icon);
        m_map->getItemsLayer()->addItem(icon);

        m_lastScreenshotTimestamp = screenshot.timestamp;
    }
}

void Route::showRoute()
{
    QTimer::singleShot(100, this, [this]()
    {
        auto target = getRect();
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

        m_map->flyTo(QGVCameraActions(m_map).scaleTo(viewRect));
    });
}

void Route::removeFromMap()
{
    m_updateTimer->stop();
    for (auto item : m_items)
    {
        m_map->getItemsLayer()->removeItem(item);
        delete item;
    }
    m_items.clear();
    m_screenshots.clear();
}
