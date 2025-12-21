//
// Created by Ian Parker on 13/11/2025.
//

#include "navigraph.h"
#include "../common/utils.h"

#include <cfloat>

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
    if (!Database::open(m_dataPath, true))
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

    sql = "SELECT type FROM approach WHERE suffix = 'A' AND airport_ident=? AND fix_ident=?";
    res = sqlite3_prepare_v2(getDB(), sql.c_str(), sql.length(), &m_findArrivalStatement, nullptr);
    if (res != SQLITE_OK)
    {
        log(ERROR, "open: Failed to prepare statement: %d: %s", res, sqlite3_errmsg(getDB()));
        return false;
    }

    sql = "SELECT type FROM approach WHERE suffix = 'D' AND airport_ident=? AND fix_ident=?";
    res = sqlite3_prepare_v2(getDB(), sql.c_str(), sql.length(), &m_findDepartureStatement, nullptr);
    if (res != SQLITE_OK)
    {
        log(ERROR, "open: Failed to prepare statement: %d: %s", res, sqlite3_errmsg(getDB()));
        return false;
    }

    sql = "SELECT airway_type FROM airway WHERE airway_name=?";
    res = sqlite3_prepare_v2(getDB(), sql.c_str(), sql.length(), &m_findAirwayStatement, nullptr);
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

vector<NavAid> NavigraphData::findNavAid(std::string name)//, NavAid &waypoint, UFC::Coordinate near)
{
    vector<NavAid> navAids;
    if (!isOpen())
    {
        return navAids;
    }

    sqlite3_bind_text(m_findNavStatement, 1, name.c_str(), -1, SQLITE_STATIC);
    while (true)
    {
        int s;
        s = sqlite3_step(m_findNavStatement);
        if (s == SQLITE_ROW)
        {
            NavAid navAid;
            navAid.coordinate.longitude = sqlite3_column_double(m_findNavStatement, 0);
            navAid.coordinate.latitude = sqlite3_column_double(m_findNavStatement, 1);
            navAid.typeStr = getString(m_findNavStatement, 2);
            navAid.name = getString(m_findNavStatement, 3);
            navAid.ident = name;

            if (navAid.typeStr.find('W') != string::npos)
            {
                navAid.type = NavAidType::WAYPOINT;
            }
            else if (navAid.typeStr.find('V') != string::npos)
            {
                navAid.type = NavAidType::VOR;
            }
            else
            {
                // Not ideal
                navAid.type = NavAidType::WAYPOINT;
            }

#if 0
            log(DEBUG, "findNavAid: %ls: %s, type=%s", navAid.coordinate.toString().c_str(), navAid.name.c_str(), navAid.typeStr.c_str());
#endif
            navAids.push_back(navAid);
        }
        else if (s == SQLITE_DONE)
        {
            break;
        }
    }
    sqlite3_reset(m_findNavStatement);

    return navAids;
}

bool NavigraphData::findDeparture(std::string airportCode, std::string name)
{
    if (!isOpen())
    {
        return false;
    }

    sqlite3_bind_text(m_findDepartureStatement, 1, airportCode.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(m_findDepartureStatement, 2, name.c_str(), -1, SQLITE_STATIC);

    bool found = false;
    int s;
    s = sqlite3_step(m_findDepartureStatement);
    if (s == SQLITE_ROW)
    {
        string type = getString(m_findDepartureStatement, 0);
#if 0
        log(DEBUG, "findDeparture: %s, %s ->  %s", airportCode.c_str(), name.c_str(), type.c_str());
#endif
        found = true;
    }
    sqlite3_reset(m_findDepartureStatement);

    return found;
}

bool NavigraphData::findArrival(std::string airportCode, std::string name)
{
    if (!isOpen())
    {
        return false;
    }

    log(DEBUG, "findArrival: Looking for %s, %s", airportCode.c_str(), name.c_str());

    sqlite3_bind_text(m_findArrivalStatement, 1, airportCode.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(m_findArrivalStatement, 2, name.c_str(), -1, SQLITE_STATIC);

    bool found = false;
    int s;
    s = sqlite3_step(m_findArrivalStatement);
    if (s == SQLITE_ROW)
    {
        string type = getString(m_findArrivalStatement, 0);
        log(DEBUG, "findArrival: %s, %s ->  %s", airportCode.c_str(), name.c_str(), type.c_str());
        found = true;
    }
    sqlite3_reset(m_findArrivalStatement);

    return found;
}

bool NavigraphData::findAirport(string code, Airport &airport)
{
    if (!isOpen())
    {
        airport.name = code;
        airport.code = code;
        return true;
    }

    sqlite3_bind_text(m_findAirportStatement, 1, code.c_str(), -1, SQLITE_STATIC);

    bool found = false;
    int s;
    s = sqlite3_step(m_findAirportStatement);
    if (s == SQLITE_ROW)
    {
        airport.code = code;
        airport.hasCoordinates = true;
        airport.coordinate.longitude = sqlite3_column_double(m_findAirportStatement, 0);
        airport.coordinate.latitude = sqlite3_column_double(m_findAirportStatement, 1);
        airport.name = getString(m_findAirportStatement, 2);
#if 0
        log(DEBUG, "findAirport: %s ->  %s: %s", code.c_str(), airport.coordinate.toString().c_str(), airport.name.c_str());
#endif
        found = true;
    }
    sqlite3_reset(m_findAirportStatement);

    return found;
}

bool NavigraphData::findNavAid(std::string name, NavAid &waypoint, UFC::Coordinate near)
{
    auto navAids = findNavAid(name);
    if (navAids.empty())
    {
        return false;
    }

    NavAid closest = navAids[0];
    double closestDistance = FLT_MAX;
    for (NavAid navAid : navAids)
    {
        auto distance = Utils::distance(navAid.coordinate, near);
        if (distance < closestDistance)
        {
            closestDistance = distance;
            closest = navAid;
        }
    }
    waypoint = closest;
    return true;
}

bool NavigraphData::findAirway(std::string ident)
{
    if (!isOpen())
    {
        return false;
    }

    sqlite3_bind_text(m_findAirwayStatement, 1, ident.c_str(), -1, SQLITE_STATIC);
    bool found = false;
    int s;
    s = sqlite3_step(m_findAirwayStatement);
    if (s == SQLITE_ROW)
    {
        string type = getString(m_findAirwayStatement, 0);
        log(DEBUG, "findAirway: %s ->  %s", ident.c_str(), type.c_str());
        found = true;
    }
    sqlite3_reset(m_findAirwayStatement);

    return found;
}

