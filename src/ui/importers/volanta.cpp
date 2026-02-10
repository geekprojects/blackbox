//
// Created by Ian Parker on 13/12/2025.
//

#include "volanta.h"

#include <fstream>
#include <filesystem>
#include <chrono>

#include <nlohmann/json.hpp>

#include "blackbox/datastore.h"
#include "blackbox/state.h"
#include "../../common/utils.h"

using namespace std;
using namespace BlackBox;
using namespace nlohmann;

VolantaImporter::VolantaImporter(DataStore* dataStore) : Logger("VolantaImporter"), m_dataStore(dataStore)
{
}

void VolantaImporter::import(std::string path)
{
    auto flightsPath = path + "/flights";
    for (const auto& entry : filesystem::directory_iterator(flightsPath))
    {
        auto filename = entry.path().filename().string();
        if (filename.starts_with("."))
        {
            continue;
        }
        log(DEBUG, "import: Flight path: %s", entry.path().c_str());
        importFlight(entry);
    }
}

string getString(const basic_json<>& j, const string& key)
{
    if (j[key].is_string())
    {
        return j[key].get<string>();
    }
    return "";
}

float getFloat(const basic_json<>& j, const string& key)
{
    if (j[key].is_number())
    {
        return j[key].get<float>();
    }
    return 0.0f;
}

float dmsToDegrees(const string &dms)
{
    float degrees = 0.0f;
    float minutes = 0.0f;
    float seconds = 0.0f;
    char d;

    sscanf(
        dms.c_str(),
        "%f° %f' %f\" %c",
        &degrees,
        &minutes,
        &seconds,
        &d);

    degrees += (minutes / 60.0f);
    degrees += (seconds / 3600.0f);
    if (d == 'S' || d == 'W')
    {
        degrees = -degrees;
    }

    return degrees;
}

uint64_t VolantaImporter::parseTimestamp(string timeStr)
{
    uint64_t timestamp = 0;
    struct tm tmStruct;
    char* s = strptime(timeStr.c_str(), "%Y-%m-%dT%H:%M:%S", &tmStruct);
    timestamp = mktime(&tmStruct) * 1000.0;

    if (s != nullptr)
    {
        int ms = 0;
        sscanf(s, ".%3dZ", &ms);
        timestamp += ms;
    }
    return timestamp;
}

State findState(vector<State>& states, uint64_t timestamp)
{
    uint64_t diff = INT64_MAX;
    State nearest = {};
    for (const auto& state : states)
    {
        uint64_t d = abs((int64_t)timestamp - (int64_t)state.timestamp);
        if (d < diff)
        {
            diff = d;
            nearest = state;
        }
    }
    return nearest;
}

void VolantaImporter::importFlight(const filesystem::directory_entry& entry)
{
    std::ifstream f(entry.path());
    json data = json::parse(f);

    /*
     * std::string origin;
     * std::string destination;
     * std::string icaoType;
     * std::string registration;
     * std::string flightId;
     * uint64_t startTime = 0;
     * std::string route;
     */

    Flight flight;
    flight.origin = getString(data, "departure.icao");
    flight.destination = getString(data, "target.icao");
    flight.icaoType = getString(data, "plane.type");
    flight.registration = getString(data, "tail.number");
    flight.flightId = getString(data, "radio.call");
    flight.route = getString(data, "flight.route");

    string dateStr = getString(data, "date.created");
    flight.startTime = parseTimestamp(dateStr);
    log(DEBUG, "importFlight: Created Date: %s -> %lld", dateStr.c_str(), flight.startTime);

    log(DEBUG, "importFlight: Origin: %s", flight.origin.c_str());
    log(DEBUG, "importFlight: Destination: %s", flight.destination.c_str());
    log(DEBUG, "importFlight: Aircraft: %s", flight.icaoType.c_str());
    log(DEBUG, "importFlight: Registration: %s", flight.registration.c_str());
    log(DEBUG, "importFlight: flight Id: %s", flight.flightId.c_str());
    log(DEBUG, "importFlight: route: %s", flight.route.c_str());

    m_dataStore->startTransaction();
    uint64_t id = m_dataStore->createFlight(flight);
    log(DEBUG, "importFlight: -> id=%lld", id);

    vector<State> states;

    for (const auto& track : data["track.points"])
    {
        /*
        * phase TEXT,"
        * event TEXT,"
        * timestamp INTEGER,"
        * sim_time INTEGER,"
        * latitude REAL,"
        * longitude REAL,"
        * altitude REAL,"
        * agl REAL,"
        * fpm REAL,"
        * fpm_average REAL,"
        * g_force REAL,"
        " pitch REAL,"
        * yaw REAL,"
        * roll REAL,"
        * ground_speed REAL,"
        * indicated_air_speed REAL"
        */
        string lat = getString(track, "lat");
        string lng = getString(track, "lng");
        //log(DEBUG, "importFlight: Lat: %s, Lng: %s", lat.c_str(), lng.c_str());

        State state;
        state.position.latitude = dmsToDegrees(lat);
        state.position.longitude = dmsToDegrees(lng);
        state.position.altitude = getFloat(track, "altitude");
        log(DEBUG, "importFlight: Position: %f, %f", state.position.latitude, state.position.longitude);

        state.agl = getFloat(track, "altitude.agl");

        if (state.agl > state.position.altitude)
        {
            continue;
        }

        state.fpm = getFloat(track, "vertical.speed");
        state.fpmAverage = state.fpm;
        state.pitch = getFloat(track, "pitch");
        state.yaw = getFloat(track, "heading.true");
        state.roll = getFloat(track, "bank");
        state.groundSpeed = getFloat(track, "speed");

        string timeStr = getString(track, "time");

        state.timestamp = parseTimestamp(timeStr);

        bool onGround = track["ground.status"].get<bool>();
        if (onGround)
        {
            if (state.groundSpeed < 0.1f)
            {
                state.flightPhase = FlightPhase::PARKED;
            }
            else
            {
                state.flightPhase = FlightPhase::TAXI;
            }
        }
        else
        {
            state.flightPhase = FlightPhase::FLIGHT;
        }

        log(DEBUG, "time: %s -> %lld", timeStr.c_str(), state.timestamp);
        states.push_back(state);
    }

    for (auto event : data["flight.events"])
    {
        if (event["landingRate"].is_number())
        {
            State state;
            // There is nothing like consistency!
            // And this Volanta data is nothing like consistent!
            state.position.latitude = getFloat(event, "latitude");
            state.position.longitude = getFloat(event, "longitude");
            state.fpm = event["landingRate"].get<float>();
            state.fpmAverage = state.fpm;
            state.timestamp = parseTimestamp(getString(event, "time"));
            state.gForce = getFloat(event, "gForce");
            state.eventType = EventType::LANDING;
            state.flightPhase = FlightPhase::LANDING;
            state.pitch = getFloat(event, "pitch");
            state.roll = getFloat(event, "roll");
            state.yaw = getFloat(event, "heading");
            state.groundSpeed = getFloat(event, "groundSpeed");

            // Fill in the blanks from the nearest track
            State nearest = findState(states, state.timestamp);
            state.position.altitude = nearest.position.altitude;
            state.agl = nearest.agl;
            log(DEBUG, "importFlight: LANDING: fpm=%0.2f, landing time=%lld, nearest=%lld", state.fpm, state.timestamp, nearest.timestamp);
            states.push_back(state);
        }
    }

    sort(states.begin(), states.end(), [](const State& a, const State& b) { return a.timestamp < b.timestamp; });


    // Now sanitise the states
    /*
    State prev;
    for (auto it = states.begin(); it != states.end(); ++it)
    {
        State& state = (*it);
        if (it != states.begin())
        {
            float diff = Utils::distance(prev.position, state.position);

        }

        prev = state;
    }
    */

    for (const auto& state : states)
    {
        m_dataStore->writeState(id, state);
    }
    m_dataStore->commitTransaction();
}
