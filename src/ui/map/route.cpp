//
// Created by Ian Parker on 14/11/2025.
//

#include "route.h"

#include <filesystem>
#include <QBrush>
#include <QPainter>
#include <QPen>
#include <QTimer>

#include <QGeoView/QGVCamera.h>

#include "landingicon.h"
#include "../blackbox.h"

using namespace std;

Route::Route(RouteMap* map, uint64_t flightId) : m_map(map), m_flightId(flightId)
{
    setFlag(QGV::ItemFlag::Clickable);

    m_planeIcon = new QImage("../data/images/plane-red.png");

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

    m_maxAltitude = 1;
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
    return Point();
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

QColor interpolate(QColor start,QColor end,double ratio)
{
    int r = (int)(ratio*start.red() + (1-ratio)*end.red());
    int g = (int)(ratio*start.green() + (1-ratio)*end.green());
    int b = (int)(ratio*start.blue() + (1-ratio)*end.blue());
    return QColor::fromRgb(r,g,b);
}

void Route::projPaint(QPainter* painter)
{
    QPen pen = QPen(QBrush(Qt::blue), 10);

    // Custom item highlight indicator
    if (isFlag(QGV::ItemFlag::Highlighted) && isFlag(QGV::ItemFlag::HighlightCustom)) {
        // We will use pen with bigger width
        pen = QPen(QBrush(Qt::black), 5);
    }

    pen.setCosmetic(true);

    auto colour1 =  QColor(0, 255, 0);
    auto colour2 =  QColor(82, 78, 221);
    //auto colour2 = QColor(87, 190, 55);
    if (m_points.size() > 1)
    {
        Point previous;
        //QPainterPath path(m_points.front().projected);
        for (auto it = m_points.begin(); it != m_points.end(); ++it)
        {
            if (it != m_points.begin())
            {
                pen.setColor(interpolate(colour2, colour1, it->altitude / m_maxAltitude));
                painter->setPen(pen);
                painter->drawLine(previous.projected, it->projected);
            }
            previous = *it;
        }
        //painter->drawPath(path);
    }

    // Custom item select indicator
    /*
    if (isSelected() && isFlag(QGV::ItemFlag::SelectCustom)) {
        // We will draw additional rect around our item
        painter->drawLine(mProjRect.topLeft(), mProjRect.bottomRight());
        painter->drawLine(mProjRect.topRight(), mProjRect.bottomLeft());
    }
    */
}

QPointF Route::projAnchor() const
{
    // This method is optional (needed flag is QGV::ItemFlag::Transformed).
    // In this case we will use center of item as base

    return m_boundingRectProjected.center();
}

QTransform Route::projTransform() const
{
    // This method is optional (needed flag is QGV::ItemFlag::Transformed).
    // Custom transformation for item.
    // In this case we rotate item by 45 degree.

    return QGV::createTransfromAzimuth(projAnchor(), 45);
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
    // This method is optional (when empty return then no tooltip).
    // Text for mouse tool tip.

    auto geo = getMap()->getProjection()->projToGeo(projPos);

    QGV::GeoPos previous;
    for (auto& point : m_points)
    {
double d = pointdistfromline2D(previous, point.position, geo);
        if (d < 0.5)
        {
char buf[1024];
            snprintf(buf, 1024, "Distance: %.2f, altitude: %0.2f", d, point.altitude);
    return buf;
        }
        previous = point.position;
    }
    return "";
}

void Route::projOnMouseClick(const QPointF& projPos)
{
    // This method is optional (needed flag is QGV::ItemFlag::Clickable).
    // Custom reaction to item single mouse click.
    // To avoid collision with item selection this code applies only if item selection disabled.
    // In this case we change opacity for item.

    if (!isSelectable()) {
        /*
        if (getOpacity() <= 0.5)
            setOpacity(1.0);
        else
            setOpacity(0.5);
*/
        qInfo() << "single click" << projPos;
    } else {
        setOpacity(1.0);
    }
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
        p.altitude = state.position.altitude;
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
            item->setGeometry(QGV::GeoPos(p.position.latitude(), p.position.longitude()), QSizeF(20, 20));
            m_items.push_back(item);
            m_map->getItemsLayer()->addItem(item);
            item->bringToFront();
        }
        if (state.flightPhase == FlightPhase::TAKE_OFF && m_lastState.flightPhase != FlightPhase::TAKE_OFF)
        {
            QImage planeIcon("../data/images/airport.png");
            auto* item = new QGVIcon();
            item->loadImage(planeIcon);
            item->setGeometry(QGV::GeoPos(p.position.latitude(), p.position.longitude()), QSizeF(20, 20));
            m_items.push_back(item);
            m_map->getItemsLayer()->addItem(item);
            item->bringToFront();
        }

        m_lastState = state;
    }

    ui->setState(m_lastState);

    if (!points.empty())
    {
        addPoints(points);

        Point point = getLastPosition();

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
    }
    m_items.clear();

}
