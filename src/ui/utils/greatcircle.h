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

    [[nodiscard]] QGV::GeoPos interpolate(double f) const;

    [[nodiscard]] std::vector<QGV::GeoPos> arcFirstPass(int numPoints) const;

    static bool calculateLongitudeDifferences(const std::vector<QGV::GeoPos> &firstPass, double &dfMaxSmallDiffLong);
    static void handleBigDifference(const std::vector<QGV::GeoPos> &firstPass, std::vector<std::vector<QGV::GeoPos>> poMulti);


    static std::vector<QGV::GeoPos> aggregateArc(const std::vector<std::vector<QGV::GeoPos>> &poMulti);

 public:
    GreatCircle(const QGV::GeoPos &start, const QGV::GeoPos &end);

    [[nodiscard]] std::vector<QGV::GeoPos> arc(int numPoints) const;
};


#endif //BLACKBOX_GREATCIRCLE_H
