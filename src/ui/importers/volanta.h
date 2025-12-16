//
// Created by Ian Parker on 13/12/2025.
//

#ifndef BLACKBOX_VOLANTA_H
#define BLACKBOX_VOLANTA_H
#include <string>
#include <__filesystem/directory_entry.h>

#include "blackbox/datastore.h"
#include "blackbox/logger.h"

class VolantaImporter : public BlackBox::Logger
{
    DataStore* m_dataStore;

 public:
    VolantaImporter(DataStore* dataStore);

    uint64_t parseTimestamp(std::string timeStr);

    void importFlight(const std::filesystem::directory_entry & entry);

    void import(std::string path);
};


#endif //BLACKBOX_VOLANTA_H
