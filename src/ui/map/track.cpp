//
// Created by Ian Parker on 14/11/2025.
//

#include "track.h"
#include "landingicon.h"
#include "screenshoticon.h"
#include "../blackbox.h"

#include <cfloat>

#include <QTimer>

#include <QGeoView/QGVCamera.h>
#include <QGeoView/QGVMapQGView.h>

using namespace std;

std::vector<std::pair<float, QColor>> Track::m_gradient =
{
    {0.000, QColor::fromHslF(23.125 / 360.0,0.88,0.51041666666666664)},
    {0.066, QColor::fromHslF(26.25 / 360.0,0.88,0.52083333333333336)},
    {0.126, QColor::fromHslF(32.5 / 360.0,0.88,0.53875)},
    {0.190, QColor::fromHslF(43 / 360.0,0.88,0.515)},
    {0.253, QColor::fromHslF(54 / 360.0,0.88,0.448)},
    {0.316, QColor::fromHslF(72 / 360.0,0.88,0.418)},
    {0.380, QColor::fromHslF(112.5 / 360.0,0.88,0.41)},
    {0.590, QColor::fromHslF(189.6551724137931 / 360.0,0.88,0.43862068965517246)},
    {0.790, QColor::fromHslF(244.82758620689657 / 360.0,0.88,0.5703448275862068)},
    {1.000, QColor::fromHslF(300 / 360.0,0.88,0.43)},
};

QColor interpolate(QColor start,QColor end,double ratio)
{
    return QColor::fromRgb(
        static_cast<int>(ratio * start.red() + (1 - ratio) * end.red()),
        static_cast<int>(ratio * start.green() + (1 - ratio) * end.green()),
        static_cast<int>(ratio * start.blue() + (1 - ratio) * end.blue()));
}

Track::Track(FlightMap* map, shared_ptr<Flight> flight) : PolyLine(map, flight)
{
    setFlag(QGV::ItemFlag::Clickable);

    m_planeIcon = new QImage(":/images/plane-red.png");

    m_positionIcon = new QGVIcon();
    m_positionIcon->loadImage(*m_planeIcon);
    m_positionIcon->setVisible(false);
    m_map->getItemsLayer()->addItem(m_positionIcon);
    m_items.push_back(m_positionIcon);

    m_updateTimer = new QTimer(this);
    connect(m_updateTimer, &QTimer::timeout, this, &Track::updateRoute);
    m_updateTimer->start(1000);
}

void Track::addPoints(std::vector<TrackPoint> points)
{
    m_points.insert(m_points.end(), points.begin(), points.end());
    printf("addPoints: Added %ld points, we now have %ld\n", points.size(), m_points.size());

    m_maxAltitude = 0.001f;
    if (!m_points.empty())
    {
        updateBoundingRect();

        for (const auto& point : m_points)
        {
            if (point.altitude > m_maxAltitude)
            {
                m_maxAltitude = point.altitude;
            }
        }
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

void Track::clear()
{
    m_points.clear();
    m_boundingRect = QGV::GeoRect();
    refresh();
}

TrackPoint Track::getLastPosition() const
{
    if (!m_points.empty())
    {
        return m_points.back();
    }
    return {};
}

void Track::projPaint(QPainter* painter)
{
    if (m_points.size() < 2)
    {
        // Nothing to show
        return;
    }

    QPen pen;
    pen.setWidth(10);
    pen.setCosmetic(true);

    auto rect = m_map->geoView()->getCamera().projRect();

    TrackPoint previous = m_points.front();
    for (auto it = m_points.begin() + 1; it != m_points.end(); ++it)
    {
        QPointF const& p1 = previous.projected;
        QPointF const& p2 = it->projected;

        QRectF const pr(p1, p2);
        if (rect.intersects(pr))
        {
            float altitude = (it->altitude + previous.altitude) / 2.0f;
            float r = altitude / 40000;
            if (r < 0)
            {
                r = 0;
            }
            if (r > 1.0)
            {
                r = 1.0;
            }

            QColor colour1 = m_gradient.front().second;
            QColor colour2;
            float previousStart = 0;
            float cr = 0.0;
            for (auto const& colour : m_gradient)
            {
                if (r < colour.first)
                {
                    colour2 = colour.second;
                    float range = colour.first - previousStart;
                    cr = (r - previousStart) / range;
                    break;
                }
                previousStart = colour.first;
                colour1 = colour.second;
            }

            pen.setColor(interpolate(colour2, colour1, cr));
            painter->setPen(pen);
            painter->drawLine(p1, p2);
        }
        previous = *it;
    }
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

QString Track::projTooltip(const QPointF& projPos) const
{
    auto geo = getMap()->getProjection()->projToGeo(projPos);

    QGV::GeoPos previous;
    TrackPoint closest;
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

void Track::updateRoute()
{
    if (m_lastTimestamp > 0 && !m_map->isVisible())
    {
        // updateRoute: Not visible! Not updating!
        printf("updateRoute: Not visible!\n");
        return;
    }

    BlackBoxUI* ui = m_map->getBlackBoxUI();
    const auto stateUpdates = ui->getDataStore().fetchUpdates(m_flight->id, m_lastTimestamp);
    if (stateUpdates.empty())
    {
        printf("updateRoute: No updates found since %lld\n", m_lastTimestamp);
        return;
    }

    vector<TrackPoint> points;
    for (const State& state : stateUpdates)
    {
        TrackPoint p;
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
            TrackPoint point = getLastPosition();

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

void Track::updateScreenshots()
{
    BlackBoxUI* ui = m_map->getBlackBoxUI();

    auto screenshots = ui->getDataStore().fetchScreenshots(m_flight->id, m_lastScreenshotTimestamp);
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

void Track::showRoute()
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

        m_map->cameraTo(QGVCameraActions(m_map).scaleTo(viewRect));
    });
}

void Track::removeFromMap()
{
    m_updateTimer->stop();
    PolyLine::removeFromMap();
    m_screenshots.clear();
}
