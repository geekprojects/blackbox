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

    static void ltrim(std::string &s)
    {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
            return !std::isspace(ch);
        }));
    }

    static void rtrim(std::string &s)
    {
        s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch)
        {
            return !std::isspace(ch);
        }).base(), s.end());
    }

    static void trim(std::string &s)
    {
        rtrim(s);
        ltrim(s);
    }
};


#endif //BLACKBOX_UTILS_H
