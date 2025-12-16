//
// Created by Ian Parker on 13/11/2025.
//

#include "navigraph.h"

using namespace std;
using namespace BlackBox;

NavigraphData::NavigraphData(std::string dataPath) : Database("Navigraph")
{
    m_dataPath = dataPath;
}

NavigraphData::~NavigraphData()
{
    NavigraphData::close();
}

bool NavigraphData::open()
{
    if (!Database::open(m_dataPath))
    {
        return false;
    }

    string sql = "SELECT lonx, laty, nav_type, name FROM nav_search WHERE ident=?";
    int res = sqlite3_prepare_v2(getDB(), sql.c_str(), sql.length(), &m_findNavStatement, nullptr);
    if (res != SQLITE_OK)
    {
        log(ERROR, "open: Failed to prepare statement: %d: %s", res, sqlite3_errmsg(getDB()));
        return false;
    }

    sql = "SELECT lonx, laty, name FROM airport WHERE ident=?";
    res = sqlite3_prepare_v2(getDB(), sql.c_str(), sql.length(), &m_findAirportStatement, nullptr);
    if (res != SQLITE_OK)
    {
        log(ERROR, "open: Failed to prepare statement: %d: %s", res, sqlite3_errmsg(getDB()));
        return false;
    }

    return true;
}

void NavigraphData::close()
{
    if (m_findNavStatement != nullptr)
    {
        sqlite3_finalize(m_findNavStatement);
    }
    if (getDB() != nullptr)
    {
        sqlite3_close(getDB());
    }
}

bool NavigraphData::findWaypoint(std::string name, Waypoint &waypoint)
{
    if (!isOpen())
    {
        return false;
    }

    sqlite3_bind_text(m_findNavStatement, 1, name.c_str(), -1, SQLITE_STATIC);
    while (true)
    {
        int s;
        s = sqlite3_step(m_findNavStatement);
        if (s == SQLITE_ROW)
        {
            float lonx = sqlite3_column_double(m_findNavStatement, 0);
            float laty = sqlite3_column_double(m_findNavStatement, 1);
            string navType = getString(m_findNavStatement, 2);
            string navName = getString(m_findNavStatement, 3);
            //string sql = "SELECT lonx, laty, nav_type, name WHERE ident=?";
            log(DEBUG, "findWaypoint: %f, %f: %s, type=%s", lonx, laty, navName.c_str(), navType.c_str());
        }
        else if (s == SQLITE_DONE)
        {
            break;
        }
    }
    sqlite3_reset(m_findNavStatement);

    return false;
}

string NavigraphData::findAirport(std::string code)
{
    if (!isOpen())
    {
        return "";
    }

    string name;
    sqlite3_bind_text(m_findAirportStatement, 1, code.c_str(), -1, SQLITE_STATIC);
    int s;
    s = sqlite3_step(m_findAirportStatement);
    if (s == SQLITE_ROW)
    {
        float lonx = sqlite3_column_double(m_findAirportStatement, 0);
        float laty = sqlite3_column_double(m_findAirportStatement, 1);
        name = getString(m_findAirportStatement, 2);
        //string sql = "SELECT lonx, laty, nav_type, name WHERE ident=?";
        log(DEBUG, "findAirport: %s ->  %f, %f: %s", code.c_str(), lonx, laty, name.c_str());
    }
    sqlite3_reset(m_findAirportStatement);

    return name;
}
