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

double Utils::distance(Coordinate c1, Coordinate c2)
{
    constexpr float earthRadiusKm = 6371;

    const auto dLat = degreesToRadians(c2.latitude-c1.latitude);
    const auto dLon = degreesToRadians(c2.longitude-c1.longitude);

    const float lat1 = degreesToRadians(c1.latitude);
    const float lat2 = degreesToRadians(c2.latitude);

    const auto a =
        sinf(dLat/2.0f) * sinf(dLat/2.0f) +
        sinf(dLon/2.0f) * sinf(dLon/2.0f) *
        cosf(lat1) * cosf(lat2);
    const auto c = 2.0f * atan2f(sqrtf(a), sqrtf(1-a));
    return earthRadiusKm * c;
}

#ifdef PLUGIN
std::string Utils::getString(const XPLMDataRef ref)
{
    int bytes = XPLMGetDatab(ref, nullptr, 0, 0);
    char buffer[bytes + 1];
    XPLMGetDatab(ref, buffer, 0, bytes);
    buffer[bytes] = '\0';
    return string(buffer);
}
#endif

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
