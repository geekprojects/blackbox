//
// Created by Ian Parker on 18/11/2025.
//


#include "blackbox/datastore.h"

using namespace std;
using namespace BlackBox;

DataStore::DataStore() : Logger("DataStore")
{
}

DataStore::~DataStore()
{
}

bool DataStore::init()
{
    int res;
    res = sqlite3_open("/Users/ian/projects/blackbox/blackbox.db", &m_db);
    if (res != SQLITE_OK)
    {
        log(ERROR, "Failed to open database");
        return false;
    }

    string sql;
    char* err;

    sql = "PRAGMA journal_mode=WAL";
    sqlite3_stmt *stmt;
    res = sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, nullptr);
    if (res != SQLITE_OK)
    {
        log(ERROR, ", init: Failed to prepare WAL statement: %s\n", err);
        return false;
    }
    res = sqlite3_step(stmt);
    if (res == SQLITE_ROW)
    {
        const unsigned char* mode = sqlite3_column_text(stmt, 0);
        log(DEBUG, "init: journal mode: %s\n", mode);
    }
    sqlite3_finalize(stmt);

    sql = "CREATE TABLE IF NOT EXISTS flights (id INTEGER PRIMARY KEY, origin TEXT, destination TEXT)";
    res = sqlite3_exec(m_db, sql.c_str(), nullptr, nullptr, &err);
    if (res != SQLITE_OK)
    {
        log(ERROR, ", init: Failed to create flights table: %s\n", err);
    }

    sql =
        "CREATE TABLE IF NOT EXISTS flight_state ("
        "    id INTEGER PRIMARY KEY,"
        "    flight_id INTEGER,"
        "    phase TEXT,"
        "    timestamp INTEGER,"
        "    latitude REAL,"
        "    longitude REAL,"
        "    altitude REAL,"
        "    agl REAL,"
        "    fpm REAL,"
        "    fpm_average REAL,"
        "    pitch REAL"
        ")";
    res = sqlite3_exec(m_db, sql.c_str(), nullptr, nullptr, &err);
    if (res != SQLITE_OK)
    {
        log(ERROR, ", init: Failed to create flight_state table: %s\n", err);
    }

    return true;
}

uint64_t DataStore::createFlight(const std::string &origin, const std::string &destination)
{
    string sql = "INSERT INTO flights (id, origin, destination) VALUES (NULL, ?, ?)";
    sqlite3_stmt *stmt;
    int res = sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, nullptr);
    if (res != SQLITE_OK)
    {
        log(ERROR, "createFlight: Failed to prepare statement: %d: %s\n", res, sqlite3_errmsg(m_db));
        return 0;
    }
    sqlite3_bind_text(stmt, 1, origin.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, destination.c_str(), -1, SQLITE_STATIC);
    res = sqlite3_step(stmt);
    if (res != SQLITE_DONE)
    {
        log(ERROR, "createFlight: Failed to insert flight: %d: %s\n", res, sqlite3_errmsg(m_db));
        return 0;
    }

    uint64_t id = sqlite3_last_insert_rowid(m_db);
    sqlite3_finalize(stmt);

    return id;
}

std::vector<Flight> DataStore::fetchFlights()
{
    string sql = "SELECT id, origin, destination FROM flights ORDER BY id ASC";
    vector<Flight> flights;
    sqlite3_stmt* stmt;
    int res = sqlite3_prepare_v2(m_db, sql.c_str(), sql.length(), &stmt, nullptr);
    if (res != SQLITE_OK)
    {
        log(ERROR, "fetchFlights: Failed to prepare statement: %d: %s\n", res, sqlite3_errmsg(m_db));
        return flights;
    }
    while (true)
    {
        int s;
        s = sqlite3_step(stmt);
        if (s == SQLITE_ROW)
        {
            Flight flight;
            flight.id = sqlite3_column_int(stmt, 0);
            flight.origin = string((char*)sqlite3_column_text(stmt, 1));
            flight.destination = string((char*)sqlite3_column_text(stmt, 2));
            flights.push_back(flight);
        }
        else if (s == SQLITE_DONE)
        {
            break;
        }
    }
    sqlite3_finalize(stmt);
    return flights;
}

void DataStore::write(uint64_t flightId, uint64_t timestamp, State &state)
{
    string sql =
        "INSERT"
        "  INTO flight_state"
        "    (id, flight_id, phase, timestamp, latitude, longitude, altitude, agl, fpm, fpm_average, pitch)"
        "  VALUES"
        "    (NULL, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";
    sqlite3_stmt *stmt;
    int res = sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, nullptr);
    if (res != SQLITE_OK)
    {
        log(ERROR, "write: Failed to prepare statement: %d: %s\n", res, sqlite3_errmsg(m_db));
        return;
    }

    sqlite3_bind_int(stmt, 1, flightId);
    sqlite3_bind_text(stmt, 2, state.getPhaseString().c_str(), state.getPhaseString().length(), SQLITE_STATIC);
    sqlite3_bind_int64(stmt, 3, timestamp);
    sqlite3_bind_double(stmt, 4, state.position.latitude);
    sqlite3_bind_double(stmt, 5, state.position.longitude);
    sqlite3_bind_double(stmt, 6, state.position.altitude);
    sqlite3_bind_double(stmt, 7, state.agl);
    sqlite3_bind_double(stmt, 8, state.fpm);
    sqlite3_bind_double(stmt, 9, state.fpmAverage);
    sqlite3_bind_double(stmt, 10, state.pitch);
    res = sqlite3_step(stmt);
    if (res != SQLITE_DONE)
    {
        log(ERROR, "write: Failed to insert state: %d: %s", res, sqlite3_errmsg(m_db));
        return;
    }

    sqlite3_finalize(stmt);
}

std::vector<State> DataStore::fetchUpdates(uint64_t flightId, uint64_t sinceTimestamp)
{
    string sql =
        "SELECT"
        "    phase,"
        "    timestamp,"
        "    latitude,"
        "    longitude,"
        "    altitude,"
        "    agl,"
        "    fpm,"
        "    fpm_average,"
        "    pitch"
        "  FROM flight_state"
        "  WHERE flight_id=? AND timestamp > ?"
        "  ORDER BY timestamp ASC"
    ;

    vector<State> states;
    sqlite3_stmt* stmt;
    int res = sqlite3_prepare_v2(m_db, sql.c_str(), sql.length(), &stmt, nullptr);
    if (res != SQLITE_OK)
    {
        log(ERROR, "fetchUpdates: Failed to prepare statement: %d: %s\n", res, sqlite3_errmsg(m_db));
        return states;
    }

    sqlite3_bind_int(stmt, 1, flightId);
    sqlite3_bind_int64(stmt, 2, sinceTimestamp);
    while (true)
    {
        int s;
        s = sqlite3_step(stmt);
        if (s == SQLITE_ROW)
        {
            State state;
            string phase = string((char*)sqlite3_column_text(stmt, 0));
            state.timestamp = sqlite3_column_int64(stmt, 1);

            state.setPhase(phase);
            state.position.latitude = sqlite3_column_double(stmt, 2);
            state.position.longitude = sqlite3_column_double(stmt, 3);
            state.position.altitude = sqlite3_column_double(stmt, 4);
            state.agl = sqlite3_column_double(stmt, 5);
            state.fpm = sqlite3_column_double(stmt, 6);
            state.fpmAverage = sqlite3_column_double(stmt, 7);
            state.pitch = sqlite3_column_double(stmt, 8);
            states.push_back(state);
            log(DEBUG, "timestamp=%lld, phase=%s", state.timestamp, phase.c_str());
        }
        else if (s == SQLITE_DONE)
        {
            break;
        }
    }
    sqlite3_finalize(stmt);
    return states;
}

void DataStore::startTransaction()
{
    sqlite3_exec(m_db, "BEGIN TRANSACTION", nullptr, nullptr, nullptr);
}

void DataStore::commitTransaction()
{
    sqlite3_exec(m_db, "COMMIT", nullptr, nullptr, nullptr);
}
