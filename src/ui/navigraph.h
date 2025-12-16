//
// Created by Ian Parker on 13/11/2025.
//

#ifndef BLACKBOX_NAVIGRAPH_H
#define BLACKBOX_NAVIGRAPH_H

#include <sqlite3.h>
#include <string>
#include <ufc/geoutils.h>

#include "blackbox/database.h"
#include "blackbox/logger.h"

struct Waypoint
{
    UFC::Coordinate coordinate;
};

class NavigraphData : public Database
{
    std::string m_dataPath;

    sqlite3_stmt* m_findNavStatement = nullptr;
    sqlite3_stmt* m_findAirportStatement = nullptr;

 public:
    explicit NavigraphData(std::string dataPath);
    ~NavigraphData() override;

    bool open();
    void close() override;

    bool findWaypoint(std::string name, Waypoint &waypoint);

    std::string findAirport(std::string code);
};


#endif //BLACKBOX_NAVIGRAPH_H
