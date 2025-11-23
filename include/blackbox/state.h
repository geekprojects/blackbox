//
// Created by Ian Parker on 18/11/2025.
//

#ifndef BLACKBOX_STATE_H
#define BLACKBOX_STATE_H

#include <ufc/geoutils.h>

enum class FlightPhase
{
    INIT,
    PARKED,
    TAXI,
    TAKE_OFF,
    FLIGHT,
    APPROACH,
    LANDING,
    CRASHED

};

struct State
{

    UFC::Coordinate position;
    float agl = 0.0f;

    float fpm = 0.0f;
    float fpmAverage = 0.0f;

    float pitch = 0.0f;

    bool paused = true;
    bool replay = false;

    bool parkingBrake = true;
    bool anyOnGround = true;
    bool allOnGround = true;
    bool inAir = true;

    FlightPhase flightPhase = FlightPhase::INIT;

    uint64_t timestamp;

    void setPhase(std::string const& phase)
    {
        using enum FlightPhase;
        if (phase == "Init")
        {
            flightPhase = INIT;
        }
        else if (phase == "Parked")
        {
            flightPhase = PARKED;
        }
        else if (phase == "Taxi")
        {
            flightPhase = TAXI;
        }
        else if (phase == "Take Off")
        {
            flightPhase = TAKE_OFF;
        }
        else if (phase == "Flight")
        {
            flightPhase = FLIGHT;
        }
        else if (phase == "Approach")
        {
            flightPhase = APPROACH;
        }
        else if (phase == "Landing")
        {
            flightPhase = LANDING;
        }
        else if (phase == "Crashed")
        {
            flightPhase = CRASHED;
        }
    }

    [[nodiscard]] std::string getPhaseString() const
    {
        switch (flightPhase)
        {
            using enum FlightPhase;
            case INIT: return "Init";
            case PARKED: return "Parked";
            case TAXI: return "Taxi";
            case TAKE_OFF: return "Take Off";
            case FLIGHT: return "Flight";
            case APPROACH: return "Approach";
            case LANDING: return "Landing";
            case CRASHED: return "Crashed";
            default: return "Unknown";
        }
    }
};

#endif //BLACKBOX_STATE_H
