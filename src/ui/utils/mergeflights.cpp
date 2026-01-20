//
// Created by Ian Parker on 03/01/2026.
//

#include "mergeflights.h"

#include "common/utils.h"

using namespace std;
using namespace BlackBox;

MergeFlights::MergeFlights(DataStore* dataStore) : Logger("MergeFlights"), m_dataStore(dataStore)
{
}

void MergeFlights::merge(vector<shared_ptr<Flight>>& flights)
{
    if (flights.size() < 2)
    {
        return;
    }

    sort(flights.begin(), flights.end(), [](const shared_ptr<Flight> &a, const shared_ptr<Flight> &b)
    {
        return a->startTime < b->startTime;
    });

    auto it = flights.begin();
    shared_ptr<Flight> first = *(it++);

    auto updates = m_dataStore->fetchUpdates(first->id, 0);
    State prevState = updates.back();

    for (; it != flights.end(); ++it)
    {
        const auto& flight = *it;
        log(DEBUG, "merge: Flight: %s", flight->toString().c_str());

        updates = m_dataStore->fetchUpdates(flight->id, 0);
        bool merged = false;
        for (auto const& state : updates)
        {
            auto dist = Utils::distance(prevState.position, state.position);
            if (dist > 0.1f)
            {
                log(DEBUG, "merge: Deleting status %llu:%llu", flight->id, state.id);
                m_dataStore->deleteState(flight->id, state.id);
            }
            else
            {
                log(DEBUG, "merge: Caught up at %llu:%llu", flight->id, state.id);
                merged = true;
                break;
            }
        }
        if (!merged)
        {
            log(WARN, "merge: Flight %llu did not catch up, aborting", flight->id);
            return;
        }
        prevState = updates.back();
    }

    auto last = flights.back();
    for (auto flight : flights)
    {
        if (flight->id == last->id)
        {
            continue;
        }

        // Renumber flight states to last flight
        m_dataStore->moveStates(flight->id, last->id);

        // Clear up flight
        m_dataStore->deleteFlight(flight->id);
    }
}
