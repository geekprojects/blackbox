//
// Created by Ian Parker on 03/01/2026.
//

#ifndef BLACKBOX_MERGEFLIGHTS_H
#define BLACKBOX_MERGEFLIGHTS_H

#include <memory>
#include <vector>

#include "blackbox/datastore.h"


class MergeFlights : BlackBox::Logger
{
    DataStore* m_dataStore;
 public:
     MergeFlights(DataStore* dataStore);
     void merge(std::vector<std::shared_ptr<Flight>>& flights);
};


#endif //BLACKBOX_MERGEFLIGHTS_H
