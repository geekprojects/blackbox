//
// Created by Ian Parker on 14/11/2025.
//

#ifndef BLACKBOX_TRACK_H
#define BLACKBOX_TRACK_H

#include <QGeoView/QGVDrawItem.h>

#include <QBrush>

#include "blackbox/state.h"
#include "routemap.h"
#include "polyline.h"

class ScreenshotIcon;

struct TrackPoint
{
    QGV::GeoPos position;
    QPointF projected;

    float altitude;
    float heading;
};


class Track : public PolyLine<TrackPoint>
{
    static std::vector<std::pair<float, QColor>> m_gradient;

    float m_maxAltitude = 0;

    State m_lastState;
    uint64_t m_lastTimestamp = 0;
    uint64_t m_lastScreenshotTimestamp = 0;
    std::vector<ScreenshotIcon*> m_screenshots;

    QImage* m_planeIcon = nullptr;
    QGVIcon* m_positionIcon = nullptr;

    QTimer* m_updateTimer;

    void projPaint(QPainter* painter) override;
    QString projTooltip(const QPointF& projPos) const override;

    void updateScreenshots();

public:
    Track(FlightMap* map, std::shared_ptr<Flight> flightId);

    //void set(std::vector<Point> points);
    void addPoints(std::vector<TrackPoint> points);
    void clear();

    TrackPoint getLastPosition() const;

    void updateRoute();

    void showRoute();

    void removeFromMap() override;
};

#endif //BLACKBOX_ROUTE_H
