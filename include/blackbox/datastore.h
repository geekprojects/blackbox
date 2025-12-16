//
// Created by Ian Parker on 18/11/2025.
//

#ifndef BLACKBOX_DATASTORE_H
#define BLACKBOX_DATASTORE_H

#include "database.h"
#include "state.h"
#include "screenshot.h"

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
};

class DataStore : public Database
{
    sqlite3_stmt* m_writeStatusStatement = nullptr;
    sqlite3_stmt* m_fetchStatusStatement = nullptr;
    sqlite3_stmt* m_insertScreenshotStatement = nullptr;
    sqlite3_stmt* m_fetchScreenshotStatement = nullptr;

    std::mutex m_insertMutex;

public:
    DataStore();
    ~DataStore() override;

    bool init(std::string dbPath);

    uint64_t createFlight(Flight &flight);
    void updateFlight(const Flight &flight);

    std::vector<Flight> fetchFlights();

    uint64_t writeState(uint64_t flightId, const State &state);
    std::vector<State> fetchUpdates(uint64_t flightId, uint64_t sinceTimestamp);

    void writeScreenshot(Screenshot &screenshot);
    std::vector<Screenshot> fetchScreenshots(uint64_t flightId, uint64_t sinceTimestamp);

    void deleteFlight(uint64_t flightId);
};


#endif //BLACKBOX_DATASTORE_H
