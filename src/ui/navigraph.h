//
// Created by Ian Parker on 13/11/2025.
//

#ifndef BLACKBOX_NAVIGRAPH_H
#define BLACKBOX_NAVIGRAPH_H

#include <sqlite3.h>
#include <string>
#include <map>

#include <ufc/utils/geoutils.h>

#include "blackbox/database.h"

enum class NavAidType
{
    UNKNOWN,
    WAYPOINT,
    VOR,
    AIRPORT,
    AIRWAY,
};

struct AirportBasicInfo
{
    std::string code;
    std::string name;
};

struct Airport : AirportBasicInfo
{
    bool hasCoordinates = false;
    UFC::Coordinate coordinate;
};

struct NavAid
{
    std::string ident;
    std::string name;
    std::string typeStr;
    NavAidType type;
    UFC::Coordinate coordinate;

    // Airway
    uint64_t sequenceNo;

    // Internal DB ids
    uint64_t sourceId;
    uint64_t nextId;
};

class NavigraphData : public Database
{
    std::string m_dataPath;

    sqlite3_stmt* m_findNavStatement = nullptr;
    sqlite3_stmt* m_findAirportStatement = nullptr;
    sqlite3_stmt* m_getAirportsStatement = nullptr;
    sqlite3_stmt* m_findDepartureStatement = nullptr;
    sqlite3_stmt* m_findArrivalStatement = nullptr;
    sqlite3_stmt* m_findAirwayStatement = nullptr;
    sqlite3_stmt* m_findNextAirwayStatement = nullptr;
    sqlite3_stmt* m_findPreviousAirwayStatement = nullptr;

    std::vector<AirportBasicInfo> m_airportInfoCache;
    std::map<std::string, Airport> m_airportCache;

 public:
    explicit NavigraphData(const std::string& dataPath);
    ~NavigraphData() override;

    bool open();

    bool findAirport(const std::string &code, Airport &airport);
    std::vector<AirportBasicInfo> getAirports();

    bool findNavAid(const std::string& name, NavAid &waypoint, UFC::Coordinate near);
    std::vector<NavAid> findNavAid(const std::string& name);

    bool findDeparture(std::string airportCode, std::string name);
    bool findArrival(const std::string &airportCode, const std::string &name);

    bool findAirway(const std::string &ident);
    bool findAirway(const std::string &ident, uint64_t entryWaypointId, NavAid &navAid, bool forward);
    bool expandAirway(const std::string& ident, uint64_t entryWaypointId, uint64_t exitWaypointId, std::vector<NavAid> &navAids);
};

#endif //BLACKBOX_NAVIGRAPH_H
