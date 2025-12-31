/*
 * Calculate Great Circle paths between two points
 *
 * This code has been adapted from the Leaflet.SmoothGeodesic library by
 * Hunter Evanoff - hunter@evanoff.dev
 * (MIT License)
 */

#include "greatcircle.h"

#include <cmath>
#include <vector>

using namespace std;
using namespace QGV;

constexpr auto D2R = M_PI / 180.0;
constexpr auto R2D = 180.0 / M_PI;

GreatCircle::GreatCircle(GeoPos start, GeoPos end)
{
    m_start.setLat(start.latitude() * D2R);
    m_start.setLon(start.longitude() * D2R);
    m_end.setLat(end.latitude() * D2R);
    m_end.setLon(end.longitude() * D2R);
    auto w = m_start.longitude() - m_end.longitude();
    auto h = m_start.latitude() - m_end.latitude();
    auto z =
        pow(sin(h / 2.0), 2) +
        cos(m_start.latitude()) *
        cos(m_end.latitude()) *
        pow(sin(w / 2.0), 2);
    m_g = 2.0 * asin(sqrt(z));
}

GeoPos GreatCircle::interpolate(double f)
{
    auto A = sin((1.0 - f) * m_g) / sin(m_g);
    auto B = sin(f * m_g) / sin(m_g);
    auto x =
        A * cos(m_start.latitude()) * cos(m_start.longitude()) +
        B * cos(m_end.latitude()) * cos(m_end.longitude());
    auto y =
        A * cos(m_start.latitude()) * sin(m_start.longitude()) +
        B * cos(m_end.latitude()) * sin(m_end.longitude());
    auto z = A * sin(m_start.latitude()) + B * sin(m_end.latitude());
    auto lat = R2D * atan2(z, sqrt(pow(x, 2.0) + pow(y, 2.0)));
    auto lon = R2D * atan2(y, x);
    return GeoPos{lat, lon};
}

vector<GeoPos> GreatCircle::arc(int numPoints)
{
    vector<GeoPos> firstPass;
    if (numPoints == 0 || numPoints <= 2)
    {
        firstPass.push_back(m_start);
        firstPass.push_back(m_end);
    }
    else
    {
        auto delta = 1.0 / (numPoints - 1);
        for (int i = 0; i < numPoints; ++i)
        {
            auto step = delta * i;
            auto pair = interpolate(step);
            firstPass.push_back(pair);
        }
    }

    auto bHasBigDiff = false;
    auto dfMaxSmallDiffLong = 0.0;
    // from http://www.gdal.org/ogr2ogr.html
    // -datelineoffset:
    // (starting with GDAL 1.10) offset from dateline in degrees (default long. = +/- 10deg, geometries within 170deg to -170deg will be splited)
    auto dfDateLineOffset = 10; //options && options.offset ? options.offset : 10;
    auto dfLeftBorderX = 180 - dfDateLineOffset;
    auto dfRightBorderX = -180 + dfDateLineOffset;
    auto dfDiffSpace = 360 - dfDateLineOffset;


    // https://github.com/OSGeo/gdal/blob/7bfb9c452a59aac958bff0c8386b891edf8154ca/gdal/ogr/ogrgeometryfactory.cpp#L2342
    for (int j = 1; j < firstPass.size(); ++j)
    {
        auto dfPrevX = firstPass[j - 1].longitude();
        auto dfX = firstPass[j].longitude();
        auto dfDiffLong = abs(dfX - dfPrevX);
        if (
            dfDiffLong > dfDiffSpace &&
            ((dfX > dfLeftBorderX && dfPrevX < dfRightBorderX) ||
             (dfPrevX > dfLeftBorderX && dfX < dfRightBorderX))
        )
        {
            bHasBigDiff = true;
        }
        else if (dfDiffLong > dfMaxSmallDiffLong)
        {
            dfMaxSmallDiffLong = dfDiffLong;
        }
    }

    vector<vector<GeoPos> > poMulti;
    if (bHasBigDiff && dfMaxSmallDiffLong < dfDateLineOffset)
    {
        vector<GeoPos> poNewLS;
        for (int k = 0; k < firstPass.size(); ++k)
        {
            auto dfX0 = firstPass[k].longitude();
            if (k > 0 && abs(dfX0 - firstPass[k - 1].longitude()) > dfDiffSpace)
            {
                auto dfX1 = firstPass[k - 1].longitude();
                auto dfY1 = firstPass[k - 1].latitude();
                auto dfX2 = firstPass[k].longitude();
                auto dfY2 = firstPass[k].latitude();
                if (
                    dfX1 > -180 &&
                    dfX1 < dfRightBorderX &&
                    dfX2 == 180 &&
                    k + 1 < firstPass.size() &&
                    firstPass[k - 1].longitude() > -180 &&
                    firstPass[k - 1].longitude() < dfRightBorderX
                )
                {
                    poNewLS.push_back(GeoPos(firstPass[k].latitude(), -180));
                    k++;
                    poNewLS.push_back(GeoPos(firstPass[k].latitude(), firstPass[k].longitude()));
                    poMulti.push_back(poNewLS);
                    continue;
                }
                else if (
                    dfX1 > dfLeftBorderX &&
                    dfX1 < 180 &&
                    dfX2 == -180 &&
                    k + 1 < firstPass.size() &&
                    firstPass[k - 1].longitude() > dfLeftBorderX &&
                    firstPass[k - 1].longitude() < 180
                )
                {
                    poNewLS.push_back({firstPass[k].latitude(), 180});
                    k++;
                    poNewLS.push_back({firstPass[k].latitude(), firstPass[k].longitude()});
                    poMulti.push_back(poNewLS);
                    continue;
                }

                if (dfX1 < dfRightBorderX && dfX2 > dfLeftBorderX)
                {
                    // swap dfX1, dfX2
                    auto tmpX = dfX1;
                    dfX1 = dfX2;
                    dfX2 = tmpX;
                    // swap dfY1, dfY2
                    auto tmpY = dfY1;
                    dfY1 = dfY2;
                    dfY2 = tmpY;
                }
                if (dfX1 > dfLeftBorderX && dfX2 < dfRightBorderX)
                {
                    dfX2 += 360;
                }

                if (dfX1 <= 180 && dfX2 >= 180 && dfX1 < dfX2)
                {
                    auto dfRatio = (180 - dfX1) / (dfX2 - dfX1);
                    auto dfY = dfRatio * dfY2 + (1 - dfRatio) * dfY1;
                    poNewLS.push_back(
                        {
                            dfY,
                            firstPass[k - 1].longitude() > dfLeftBorderX ? 180.0f : -180.0f
                        });
                    poMulti.push_back(poNewLS);

                    vector<GeoPos> ls2;
                    ls2.push_back(
                        {
                            dfY,
                            firstPass[k - 1].longitude() > dfLeftBorderX ? -180.0f : 180.0f,
                        });
                    ls2.push_back({firstPass[k].latitude(), dfX0});
                    poMulti.push_back(ls2);
                }
                else
                {
                    poMulti.push_back(poNewLS);
                    vector<GeoPos> ls2;
                    ls2.push_back({firstPass[k].latitude(), dfX0});
                    poMulti.push_back(ls2);
                }
            }
            else
            {
                poNewLS.push_back(firstPass[k]);
                poMulti.push_back(poNewLS);
            }
        }
    }
    else
    {
        // add normally
        vector<GeoPos> poNewLS0;
        for (int l = 0; l < firstPass.size(); ++l)
        {
            poNewLS0.push_back(firstPass[l]);
        }
        poMulti.push_back(poNewLS0);
    }

    vector<GeoPos> arc;
    for (auto poLine : poMulti)
    {
        for (auto point : poLine)
        {
            //printf("%f, %f\n", point.latitude(), point.longitude());
            arc.push_back(point);
        }
    }
    return arc;
}
