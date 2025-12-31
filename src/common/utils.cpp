//
// Created by Ian Parker on 29/11/2025.
//

#include "utils.h"

using namespace std;
using namespace UFC;

float Utils::degreesToRadians(float degrees)
{
    return degrees * M_PI / 180.0f;
}

float Utils::radiansToDegreens(float radians)
{
    return radians * 180.0f / M_PI;
}

double Utils::distance(double lat1, double lon1, double lat2, double lon2)
{
    constexpr float earthRadiusKm = 6371;

    const auto dLat = degreesToRadians(lat2-lat1);
    const auto dLon = degreesToRadians(lon2-lon1);

    lat1 = degreesToRadians(lat1);
    lat2 = degreesToRadians(lat2);

    const auto a =
        sinf(dLat/2.0f) * sinf(dLat/2.0f) +
        sinf(dLon/2.0f) * sinf(dLon/2.0f) *
        cosf(lat1) * cosf(lat2);
    const auto c = 2.0f * atan2f(sqrtf(a), sqrtf(1-a));
    return earthRadiusKm * c;
}

double Utils::distance(Coordinate c1, Coordinate c2)
{
    return distance(c1.latitude, c1.longitude, c2.latitude, c2.longitude);
}

double Utils::angleFromCoordinate(Coordinate coord1, Coordinate coord2)
{
    double phi1 = degreesToRadians(coord1.latitude);
    double phi2 = degreesToRadians(coord2.latitude);

    double dLon = degreesToRadians(coord2.longitude - coord1.longitude);
    double y = sin(dLon) * cos(phi2);
    double x =
        cos(phi1) * sin(phi2) -
        sin(phi1) * cos(phi2) *
        cos(dLon);

    return atan2(y, x);
}

vector<string> Utils::splitString(string line)
{
    vector<string> parts;

    Utils::trim(line);

    while (!line.empty())
    {
        size_t pos = line.find(' ');
        if (pos == string::npos)
        {
            pos = line.find('\t');
        }
        if (pos == string::npos)
        {
            pos = line.length();
            if (pos == 0)
            {
                break;
            }
        }
        if (pos >= 1)
        {
            string part = line.substr(0, pos);
            Utils::trim(part);
            parts.push_back(part);
        }
        if (pos == line.length())
        {
            break;
        }
        line = line.substr(pos + 1);
    }

    return parts;
}
