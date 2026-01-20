//
// Created by Ian Parker on 25/11/2025.
//

#ifndef BLACKBOX_BLACKBOX_H
#define BLACKBOX_BLACKBOX_H

#include <QApplication>

#include "navigraph.h"
#include "blackbox/datastore.h"

class MainWindow;
class FlightsWindow;

class BlackBoxUI : public QApplication
{
    MainWindow* m_mainWindow = nullptr;
    FlightsWindow* m_flightsWindow = nullptr;

    DataStore m_dataStore;
    std::shared_ptr<NavigraphData> m_navigraph;

    State m_latestState;

    std::vector<std::shared_ptr<Flight>> m_flights;
    std::map<uint64_t, std::shared_ptr<Flight>> m_flightIndex;
    std::shared_ptr<Flight> m_currentFlight;

    void setupCachedNetworkAccessManager();

 public:
    BlackBoxUI(int argc, char** argv);
    ~BlackBoxUI() override = default;

    int run();

    void openFlightsWindow() const;

    void updateFlights();
    std::shared_ptr<Flight> getCurrentFlight() { return m_currentFlight; }
    void setCurrentFlightId(uint64_t flightId)
    {
        auto it = m_flightIndex.find(flightId);
        if (it != m_flightIndex.end())
        {
            m_currentFlight = it->second;
        }
    }

    std::shared_ptr<Flight> getFlight(uint64_t flightId) const
    {
        auto it = m_flightIndex.find(flightId);
        if (it != m_flightIndex.end())
        {
            return it->second;
        }
        return nullptr;
    }
    std::vector<std::shared_ptr<Flight>> getFlights() const { return m_flights; }
    void deleteFlight(std::shared_ptr<Flight> flight);

    void setState(const State& state);
    const State& getState() const { return m_latestState; }

    MainWindow* getMainWindow() const { return m_mainWindow; }
    DataStore& getDataStore() { return m_dataStore; }
    std::shared_ptr<NavigraphData> getNavigraph() { return m_navigraph; }
};

#endif //BLACKBOX_BLACKBOX_H
