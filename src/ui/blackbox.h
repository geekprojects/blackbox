//
// Created by Ian Parker on 25/11/2025.
//

#ifndef BLACKBOX_BLACKBOX_H
#define BLACKBOX_BLACKBOX_H

#include <QApplication>

#include "blackbox/datastore.h"

class MainWindow;

class BlackBoxUI
{
    QApplication m_app;
    MainWindow* m_mainWindow = nullptr;

    DataStore m_dataStore;

    State m_latestState;

    std::map<uint64_t, Flight> m_flights;
    Flight m_currentFlight;

 public:
    BlackBoxUI(int argc, char** argv);
    ~BlackBoxUI() = default;

    int run();

    void updateFlights();
    Flight& getCurrentFlight() { return m_currentFlight; }
    void setCurrentFlightId(uint64_t flightId) { m_currentFlight = m_flights.at(flightId); }
    std::map<uint64_t, Flight> getFlights() const { return m_flights; }

    void setState(const State& state);
    const State& getState() const { return m_latestState; }

    DataStore& getDataStore() { return m_dataStore; }
};

#endif //BLACKBOX_BLACKBOX_H
