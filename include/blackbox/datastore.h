//
// Created by Ian Parker on 18/11/2025.
//

#ifndef BLACKBOX_DATASTORE_H
#define BLACKBOX_DATASTORE_H

#include <sqlite3.h>

#include "state.h"
#include "logger.h"

struct Flight
{
    uint64_t id = 0;
    std::string origin;
    std::string destination;
    std::string icaoType;
    std::string flightId;
    uint64_t startTime = 0;
};

class DataStore : BlackBox::Logger
{
    sqlite3* m_db = nullptr;
    sqlite3_stmt* m_writeStatusStatement = nullptr;
    sqlite3_stmt* m_fetchStatusStatement = nullptr;

 public:
    DataStore();
    ~DataStore();

    bool init();

    uint64_t createFlight(Flight &flight);
    void updateFlight(const Flight &flight);

    std::vector<Flight> fetchFlights();

    void writeState(uint64_t flightId, const State &state);
    std::vector<State> fetchUpdates(uint64_t flightId, uint64_t sinceTimestamp);

    void startTransaction();
    void commitTransaction();

    void deleteFlight(uint64_t flightId);
};


#endif //BLACKBOX_DATASTORE_H
