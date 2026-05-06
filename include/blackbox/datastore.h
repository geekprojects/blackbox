//
// Created by Ian Parker on 18/11/2025.
//

#ifndef BLACKBOX_DATASTORE_H
#define BLACKBOX_DATASTORE_H

#include "database.h"
#include "state.h"
#include "screenshot.h"

#include <map>

struct Flight
{
    uint64_t id = 0;
    std::string origin;
    std::string destination;
    std::string icaoType;
    std::string registration;
    std::string flightId;
    uint64_t startTime = 0;
    std::string route;

    // These values are not stored in the DB
    uint64_t stateCount = 0;
    double landingRate = 0;

    std::string toString() const
    {
        return std::to_string(id) + ": " + origin + " -> " + destination;
    }
};

class DataStore : public Database
{
    sqlite3_stmt* m_writeStatusStatement = nullptr;
    sqlite3_stmt* m_fetchStatusStatement = nullptr;
    sqlite3_stmt* m_countStatusStatement = nullptr;
    sqlite3_stmt* m_deleteStatusStatement = nullptr;
    sqlite3_stmt* m_moveStatusStatement = nullptr;
    sqlite3_stmt* m_insertScreenshotStatement = nullptr;
    sqlite3_stmt* m_fetchScreenshotStatement = nullptr;
    sqlite3_stmt* m_landingStatement = nullptr;

    std::map<std::string, sqlite3_stmt*> m_statementCache;

    std::mutex m_insertMutex;


public:
    DataStore();
    ~DataStore() override = default;

    bool init(const std::string &dbPath);

    uint64_t createFlight(Flight &flight);
    void updateFlight(const Flight &flight);
    void updateFlight(const Flight &flight, const std::string &field, const std::string &value);

    std::vector<Flight> fetchFlights();

    uint64_t writeState(uint64_t flightId, const State &state);
    std::vector<State> fetchUpdates(uint64_t flightId, uint64_t sinceTimestamp);
    uint64_t countUpdates(uint64_t flightId);
    double getLandingRate(uint64_t flightId);
    void deleteState(uint64_t flightId, uint64_t stateId);
    void moveStates(uint64_t from, uint64_t to);

    void writeScreenshot(Screenshot &screenshot);
    std::vector<Screenshot> fetchScreenshots(uint64_t flightId, uint64_t sinceTimestamp);

    void deleteFlight(uint64_t flightId);
};


#endif //BLACKBOX_DATASTORE_H
