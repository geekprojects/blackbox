//
// Created by Ian Parker on 29/11/2025.
//

#ifndef BLACKBOX_UTILS_H
#define BLACKBOX_UTILS_H

#include <ufc/geoutils.h>

#ifdef PLUGIN
#include "XPLMDataAccess.h"
#endif

class Utils
{
 public:
    static float degreesToRadians(float degrees);
    static float radiansToDegreens(float radians);
    static double distance(UFC::Coordinate c1, UFC::Coordinate c2);
    static double angleFromCoordinate(UFC::Coordinate coord1, UFC::Coordinate coord2);

#ifdef PLUGIN
#endif
};


#endif //BLACKBOX_UTILS_H
