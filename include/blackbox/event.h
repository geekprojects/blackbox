//
// Created by Ian Parker on 19/11/2025.
//

#ifndef BLACKBOX_EVENT_H
#define BLACKBOX_EVENT_H

#include "state.h"

enum class EventType
{
    NEW_FLIGHT,
    PHASE_CHANGE,
    UPDATE,
    LANDING
};

struct FlightDetails
{
    char icaoType[10];
    char origin[10];
    char destination[10];
};

struct Event
{
    EventType type;

    FlightDetails details;
    State state;
};

#endif //BLACKBOX_EVENT_H
