//
// Created by Ian Parker on 06/12/2025.
//

#ifndef BLACKBOX_SCREENSHOT_H
#define BLACKBOX_SCREENSHOT_H

#include <string>

#include <ufc/geoutils.h>

struct Screenshot
{
    uint64_t id;
    uint64_t flightId;
    uint64_t flightStateId;
    uint64_t timestamp;
    std::string path;

    // Not directly stored in the screenshots table
    UFC::Coordinate position;
};

#endif //BLACKBOX_SCREENSHOT_H
