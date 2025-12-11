//
// Created by Ian Parker on 09/12/2025.
//

#ifndef BLACKBOX_AIRCRAFTTYPES_H
#define BLACKBOX_AIRCRAFTTYPES_H

#include <map>
#include <string>

class AircraftTypes
{
    static std::map<std::string, std::string> m_types;

 public:
    static std::string getName(std::string icaoType);
};


#endif //BLACKBOX_AIRCRAFTTYPES_H
