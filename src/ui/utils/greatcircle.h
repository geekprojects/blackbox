/*
* Calculate Great Circle paths between two points
 *
 * This code has been adapted from the Leaflet.SmoothGeodesic library by
 * Hunter Evanoff - hunter@evanoff.dev
 * (MIT License)
 */

#ifndef BLACKBOX_GREATCIRCLE_H
#define BLACKBOX_GREATCIRCLE_H

#include <QGeoView/QGVGlobal.h>

class GreatCircle
{
    QGV::GeoPos m_start;
    QGV::GeoPos m_end;
    double m_g;

 public:
    GreatCircle(QGV::GeoPos start, QGV::GeoPos end);

    QGV::GeoPos interpolate(double f);

    std::vector<QGV::GeoPos> arc(int numPoints);
};


#endif //BLACKBOX_GREATCIRCLE_H
