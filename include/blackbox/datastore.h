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
    uint64_t id;
    std::string origin;
    std::string destination;
};

class DataStore : BlackBox::Logger
{
    sqlite3* m_db = nullptr;

 public:
    DataStore();
    ~DataStore();

    bool init();

    uint64_t createFlight(const std::string& origin, const std::string& destination);
    std::vector<Flight> fetchFlights();

    void write(uint64_t flightId, uint64_t timestamp, State &state);
    std::vector<State> fetchUpdates(uint64_t flightId, uint64_t sinceTimestamp);

    void startTransaction();
    void commitTransaction();
};


#endif //BLACKBOX_DATASTORE_H
