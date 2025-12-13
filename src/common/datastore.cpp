//
// Created by Ian Parker on 18/11/2025.
//


#include "blackbox/datastore.h"

#include "blackbox/screenshot.h"

using namespace std;
using namespace BlackBox;

string getString(sqlite3_stmt* stmt, int col)
{
    const unsigned char* str = sqlite3_column_text(stmt, col);
    if (str == nullptr)
    {
        return "";
    }
    return {reinterpret_cast<const char*>(str)};
}

DataStore::DataStore() : Logger("DataStore")
{
}

DataStore::~DataStore()
{
    if (m_writeStatusStatement != nullptr)
    {
        sqlite3_finalize(m_writeStatusStatement);
    }
    if (m_fetchStatusStatement != nullptr)
    {
        sqlite3_finalize(m_fetchStatusStatement);
    }
    if (m_insertScreenshotStatement != nullptr)
    {
        sqlite3_finalize(m_insertScreenshotStatement);
    }

    if (m_db != nullptr)
    {
        sqlite3_close(m_db);
    }
}

bool DataStore::init(string dbPath)
{
    int res = sqlite3_open(dbPath.c_str(), &m_db);
    if (res != SQLITE_OK)
    {
        log(ERROR, "Failed to open database");
        return false;
    }

    string sql;
    char* err;

    // Make sure Write Ahead Logging is enabled
    sql = "PRAGMA journal_mode=WAL";
    sqlite3_stmt *stmt;
    res = sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, nullptr);
    if (res != SQLITE_OK)
    {
        log(ERROR, "init: Failed to prepare WAL statement: %d", res);
        return false;
    }
    res = sqlite3_step(stmt);
    if (res == SQLITE_ROW)
    {
        string mode = getString(stmt, 0);
        log(DEBUG, "init: journal mode: %s", mode.c_str());
    }
    sqlite3_finalize(stmt);

    sql =
        "CREATE TABLE IF NOT EXISTS flights ("
        "    id INTEGER PRIMARY KEY,"
        "    origin TEXT,"
        "    destination TEXT,"
        "    aircraft_type TEXT,"
        "    aircraft_registration TEXT,"
        "    flight_code TEXT,"
        "    start_time INTEGER"
        ")";
    res = sqlite3_exec(m_db, sql.c_str(), nullptr, nullptr, &err);
    if (res != SQLITE_OK)
    {
        log(ERROR, "init: Failed to create flights table: %s", err);
        return false;
    }

    sql =
        "CREATE TABLE IF NOT EXISTS flight_state ("
        "    id INTEGER PRIMARY KEY,"
        "    flight_id INTEGER,"
        "    phase TEXT,"
        "    event TEXT,"
        "    timestamp INTEGER,"
        "    sim_time INTEGER,"
        "    latitude REAL,"
        "    longitude REAL,"
        "    altitude REAL,"
        "    agl REAL,"
        "    fpm REAL,"
        "    fpm_average REAL,"
        "    g_force REAL,"
        "    pitch REAL,"
        "    yaw REAL,"
        "    roll REAL,"
        "    ground_speed REAL,"
        "    indicated_air_speed REAL"
        ")";
    res = sqlite3_exec(m_db, sql.c_str(), nullptr, nullptr, &err);
    if (res != SQLITE_OK)
    {
        log(ERROR, "init: Failed to create flight_state table: %s", err);
        return false;
    }

    sql = "CREATE INDEX IF NOT EXISTS flight_state_by_id ON flight_state (flight_id)";
    res = sqlite3_exec(m_db, sql.c_str(), nullptr, nullptr, &err);
    if (res != SQLITE_OK)
    {
        log(ERROR, "init: Failed to create flight_state index: %s", err);
        return false;
    }

    sql =
        "INSERT"
        "  INTO flight_state"
        "    (id, flight_id, phase, event, timestamp, sim_time, latitude, longitude, altitude, agl, fpm, fpm_average, g_force, pitch, yaw, roll, ground_speed, indicated_air_speed)"
        "  VALUES"
        "    (NULL, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";
    res = sqlite3_prepare_v2(m_db, sql.c_str(), -1, &m_writeStatusStatement, nullptr);
    if (res != SQLITE_OK)
    {
        log(ERROR, "init: Failed to prepare statement: %d: %s", res, sqlite3_errmsg(m_db));
        return false;
    }

    sql =
        "CREATE TABLE IF NOT EXISTS screenshots ("
        "    id INTEGER PRIMARY KEY,"
        "    flight_id INTEGER,"
        "    flight_state_id INTEGER,"
        "    timestamp INTEGER,"
        "    path TEXT"
    ")";
    res = sqlite3_exec(m_db, sql.c_str(), nullptr, nullptr, &err);
    if (res != SQLITE_OK)
    {
        log(ERROR, "init: Failed to create screenshots table: %s", err);
        return false;
    }

    sql =
        "SELECT"
        "    phase,"
        "    event,"
        "    timestamp,"
        "    sim_time,"
        "    latitude,"
        "    longitude,"
        "    altitude,"
        "    agl,"
        "    fpm,"
        "    fpm_average,"
        "    g_force,"
        "    pitch,"
        "    yaw,"
        "    roll,"
        "    ground_speed,"
        "    indicated_air_speed"
        "  FROM flight_state"
        "  WHERE flight_id=? AND timestamp > ?"
        "  ORDER BY timestamp ASC";
    res = sqlite3_prepare_v2(m_db, sql.c_str(), -1, &m_fetchStatusStatement, nullptr);
    if (res != SQLITE_OK)
    {
        log(ERROR, "init: Failed to prepare statement: %d: %s", res, sqlite3_errmsg(m_db));
        return false;
    }

    sql =
        "INSERT INTO screenshots ("
        "    id,"
        "    flight_id,"
        "    flight_state_id,"
        "    timestamp,"
        "    path"
        "  ) VALUES ("
        "    NULL, ?, ?, ? ,?"
        ")";
    res = sqlite3_prepare_v2(m_db, sql.c_str(), -1, &m_insertScreenshotStatement, nullptr);
    if (res != SQLITE_OK)
    {
        log(ERROR, "init: Failed to prepare statement: %d: %s", res, sqlite3_errmsg(m_db));
        return false;
    }

    sql =
        "SELECT "
        "    ss.id,"
        "    ss.flight_id,"
        "    ss.flight_state_id,"
        "    ss.timestamp,"
        "    ss.path,"
        "    fs.latitude,"
        "    fs.longitude,"
        "    fs.altitude"
        "  FROM screenshots ss"
        "  JOIN flight_state fs ON fs.id = ss.flight_state_id"
        "  WHERE"
        "    ss.flight_id = ? AND"
        "    ss.timestamp > ?"
        "  ORDER BY ss.timestamp ASC";
    res = sqlite3_prepare_v2(m_db, sql.c_str(), -1, &m_fetchScreenshotStatement, nullptr);
    if (res != SQLITE_OK)
    {
        log(ERROR, "init: Failed to prepare statement: %d: %s", res, sqlite3_errmsg(m_db));
        return false;
    }

    return true;
}

uint64_t DataStore::createFlight(Flight& flight)
{
    const string sql =
        "INSERT INTO flights"
        "    (id, origin, destination, aircraft_type, aircraft_registration, flight_code, start_time)"
        "  VALUES"
        "    (NULL, ?, ?, ?, ?, ?, ?)";
    sqlite3_stmt *stmt;
    int res = sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, nullptr);
    if (res != SQLITE_OK)
    {
        log(ERROR, "createFlight: Failed to prepare statement: %d: %s", res, sqlite3_errmsg(m_db));
        return 0;
    }
    sqlite3_bind_text(stmt, 1, flight.origin.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, flight.destination.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, flight.icaoType.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, flight.registration.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 5, flight.flightId.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int64(stmt, 6, flight.startTime);

    scoped_lock lock(m_insertMutex);
    res = sqlite3_step(stmt);
    if (res != SQLITE_DONE)
    {
        log(ERROR, "createFlight: Failed to insert flight: %d: %s", res, sqlite3_errmsg(m_db));
        return 0;
    }

    flight.id = sqlite3_last_insert_rowid(m_db);
    sqlite3_finalize(stmt);

    return flight.id;
}

void DataStore::updateFlight(const Flight& flight)
{
    string sql = "UPDATE flights SET origin=?, destination=?, aircraft_type=?, aircraft_registration=?, flight_code=? WHERE id=?";
    sqlite3_stmt *stmt;
    int res = sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, nullptr);
    if (res != SQLITE_OK)
    {
        log(ERROR, "updateFlight: Failed to prepare statement: %d: %s", res, sqlite3_errmsg(m_db));
        return;
    }

    sqlite3_bind_text(stmt, 1, flight.origin.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, flight.destination.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, flight.icaoType.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, flight.registration.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 5, flight.flightId.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int64(stmt, 6, flight.id);

    res = sqlite3_step(stmt);
    if (res != SQLITE_DONE)
    {
        log(ERROR, "updateFlight: Failed to update flight: %d: %s", res, sqlite3_errmsg(m_db));
    }
    sqlite3_finalize(stmt);
}


std::vector<Flight> DataStore::fetchFlights()
{
    string sql = "SELECT id, origin, destination, aircraft_type, aircraft_registration, flight_code, start_time FROM flights ORDER BY id ASC";
    vector<Flight> flights;
    sqlite3_stmt* stmt;
    int res = sqlite3_prepare_v2(m_db, sql.c_str(), sql.length(), &stmt, nullptr);
    if (res != SQLITE_OK)
    {
        log(ERROR, "fetchFlights: Failed to prepare statement: %d: %s", res, sqlite3_errmsg(m_db));
        return flights;
    }
    while (true)
    {
        int s;
        s = sqlite3_step(stmt);
        if (s == SQLITE_ROW)
        {
            Flight flight;
            flight.id = sqlite3_column_int64(stmt, 0);
            flight.origin = getString(stmt, 1);
            flight.destination = getString(stmt, 2);
            flight.icaoType = getString(stmt, 3);
            flight.registration = getString(stmt, 4);
            flight.flightId = getString(stmt, 5);
            flight.startTime = sqlite3_column_int64(stmt, 6);
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

uint64_t DataStore::writeState(uint64_t flightId, const State &state)
{
    string phaseString = state.getPhaseString();
    string eventString = state.getEventString();

    int col = 1;
    sqlite3_bind_int64(m_writeStatusStatement, col++, flightId);
    sqlite3_bind_text(m_writeStatusStatement, col++, phaseString.c_str(), phaseString.length(), SQLITE_STATIC);
    sqlite3_bind_text(m_writeStatusStatement, col++, eventString.c_str(), eventString.length(), SQLITE_STATIC);
    sqlite3_bind_int64(m_writeStatusStatement, col++, state.timestamp);
    sqlite3_bind_int64(m_writeStatusStatement, col++, state.simTime);
    sqlite3_bind_double(m_writeStatusStatement, col++, state.position.latitude);
    sqlite3_bind_double(m_writeStatusStatement, col++, state.position.longitude);
    sqlite3_bind_double(m_writeStatusStatement, col++, state.position.altitude);
    sqlite3_bind_double(m_writeStatusStatement, col++, state.agl);
    sqlite3_bind_double(m_writeStatusStatement, col++, state.fpm);
    sqlite3_bind_double(m_writeStatusStatement, col++, state.fpmAverage);
    sqlite3_bind_double(m_writeStatusStatement, col++, state.gForce);
    sqlite3_bind_double(m_writeStatusStatement, col++, state.pitch);
    sqlite3_bind_double(m_writeStatusStatement, col++, state.yaw);
    sqlite3_bind_double(m_writeStatusStatement, col++, state.roll);
    sqlite3_bind_double(m_writeStatusStatement, col++, state.groundSpeed);
    sqlite3_bind_double(m_writeStatusStatement, col++, state.indicatedAirSpeed);

    scoped_lock lock(m_insertMutex);
    int res = sqlite3_step(m_writeStatusStatement);
    if (res != SQLITE_DONE)
    {
        log(ERROR, "write: Failed to insert state: %d: %s", res, sqlite3_errmsg(m_db));
        return 0;
    }
    auto rowid = sqlite3_last_insert_rowid(m_db);
    sqlite3_reset(m_writeStatusStatement);

    return rowid;
}

std::vector<State> DataStore::fetchUpdates(uint64_t flightId, uint64_t sinceTimestamp)
{
    vector<State> states;

    sqlite3_bind_int64(m_fetchStatusStatement, 1, flightId);
    sqlite3_bind_int64(m_fetchStatusStatement, 2, sinceTimestamp);
    while (true)
    {
        int s;
        s = sqlite3_step(m_fetchStatusStatement);
        if (s == SQLITE_ROW)
        {
            int col = 0;
            State state;
            string phase = getString(m_fetchStatusStatement, col++);
            state.setPhase(phase);
            string event = getString(m_fetchStatusStatement, col++);
            state.setEventType(event);

            state.timestamp = sqlite3_column_int64(m_fetchStatusStatement, col++);
            state.simTime = sqlite3_column_int64(m_fetchStatusStatement, col++);

            state.position.latitude = sqlite3_column_double(m_fetchStatusStatement, col++);
            state.position.longitude = sqlite3_column_double(m_fetchStatusStatement, col++);
            state.position.altitude = sqlite3_column_double(m_fetchStatusStatement, col++);
            state.agl = sqlite3_column_double(m_fetchStatusStatement, col++);
            state.fpm = sqlite3_column_double(m_fetchStatusStatement, col++);
            state.fpmAverage = sqlite3_column_double(m_fetchStatusStatement, col++);
            state.gForce = sqlite3_column_double(m_fetchStatusStatement, col++);
            state.pitch = sqlite3_column_double(m_fetchStatusStatement, col++);
            state.yaw = sqlite3_column_double(m_fetchStatusStatement, col++);
            state.roll = sqlite3_column_double(m_fetchStatusStatement, col++);
            state.groundSpeed = sqlite3_column_double(m_fetchStatusStatement, col++);
            state.indicatedAirSpeed = sqlite3_column_double(m_fetchStatusStatement, col++);
            states.push_back(state);
        }
        else if (s == SQLITE_DONE)
        {
            break;
        }
    }

    sqlite3_reset(m_fetchStatusStatement);

    return states;
}

void DataStore::writeScreenshot(Screenshot& screenshot)
{
    sqlite3_bind_int64(m_insertScreenshotStatement, 1, screenshot.flightId);
    sqlite3_bind_int64(m_insertScreenshotStatement, 2, screenshot.flightStateId);
    sqlite3_bind_int64(m_insertScreenshotStatement, 3, screenshot.timestamp);
    sqlite3_bind_text(m_insertScreenshotStatement, 4, screenshot.path.c_str(), screenshot.path.length(), SQLITE_STATIC);

    scoped_lock lock(m_insertMutex);
    auto res = sqlite3_step(m_insertScreenshotStatement);
    if (res != SQLITE_DONE)
    {
        log(ERROR, "createFlight: Failed to insert flight: %d: %s", res, sqlite3_errmsg(m_db));
        return;
    }

    screenshot.id = sqlite3_last_insert_rowid(m_db);
    sqlite3_reset(m_insertScreenshotStatement);
}

vector<Screenshot> DataStore::fetchScreenshots(uint64_t flightId, uint64_t sinceTimestamp)
{
    vector<Screenshot> screenshots;
    vector<State> states;

    sqlite3_bind_int64(m_fetchScreenshotStatement, 1, flightId);
    sqlite3_bind_int64(m_fetchScreenshotStatement, 2, sinceTimestamp);
    while (true)
    {
        int s;
        s = sqlite3_step(m_fetchScreenshotStatement);
        if (s == SQLITE_ROW)
        {
            Screenshot screenshot;
            screenshot.id = sqlite3_column_int64(m_fetchScreenshotStatement, 0);
            screenshot.flightId = sqlite3_column_int64(m_fetchScreenshotStatement, 1);
            screenshot.flightStateId = sqlite3_column_int64(m_fetchScreenshotStatement, 2);
            screenshot.timestamp = sqlite3_column_int64(m_fetchScreenshotStatement, 3);
            screenshot.path = getString(m_fetchScreenshotStatement, 4);
            screenshot.position.latitude = sqlite3_column_double(m_fetchScreenshotStatement, 5);
            screenshot.position.longitude = sqlite3_column_double(m_fetchScreenshotStatement, 6);
            screenshot.position.altitude = sqlite3_column_double(m_fetchScreenshotStatement, 7);
            screenshots.push_back(screenshot);
        }
        else if (s == SQLITE_DONE)
        {
            break;
        }
    }

    sqlite3_reset(m_fetchScreenshotStatement);

    return screenshots;
}

void DataStore::startTransaction()
{
    int res = sqlite3_exec(m_db, "BEGIN TRANSACTION", nullptr, nullptr, nullptr);
    if (res != SQLITE_OK)
    {
        log(ERROR, "startTransaction: Failed to start transaction: %d: %s", res, sqlite3_errmsg(m_db));
    }
}

void DataStore::commitTransaction()
{
    int res = sqlite3_exec(m_db, "COMMIT", nullptr, nullptr, nullptr);
    if (res != SQLITE_OK)
    {
        log(ERROR, "commitTransaction: Failed to commit transaction: %d: %s", res, sqlite3_errmsg(m_db));
    }
}

void DataStore::deleteFlight(uint64_t flightId)
{
    log(DEBUG, "deleteFlight: Deleting flightId: %d", flightId);
    string sql = "DELETE FROM flight_state WHERE flight_id=?";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(m_db, sql.c_str(), sql.length(), &stmt, nullptr);
    sqlite3_bind_int64(stmt, 1, flightId);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    sql = "DELETE FROM flights WHERE id=?";
    sqlite3_prepare_v2(m_db, sql.c_str(), sql.length(), &stmt, nullptr);
    sqlite3_bind_int64(stmt, 1, flightId);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    log(DEBUG, "deleteFlight: Deleted flightId: %d", flightId);
}
