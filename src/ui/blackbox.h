//
// Created by Ian Parker on 25/11/2025.
//

#ifndef BLACKBOX_BLACKBOX_H
#define BLACKBOX_BLACKBOX_H

#include <QApplication>

#include "navigraph.h"
#include "blackbox/datastore.h"

class QSettings;
class MainWindow;
class FlightsWindow;

class BlackBoxUI : public QApplication
{
    QSettings* m_settings;

    MainWindow* m_mainWindow = nullptr;
    FlightsWindow* m_flightsWindow = nullptr;

    DataStore m_dataStore;
    std::shared_ptr<NavigraphData> m_navigraph;

    State m_latestState;

    std::vector<std::shared_ptr<Flight>> m_flights;
    std::map<uint64_t, std::shared_ptr<Flight>> m_flightIndex;

    std::vector<std::shared_ptr<Flight>> m_currentFlights;
    std::shared_ptr<Flight> m_selectedFlight;

    void setupCachedNetworkAccessManager();

 public:
    BlackBoxUI(int argc, char** argv);
    ~BlackBoxUI() override;

    int run();

    void openFlightsWindow() const;

    void updateFlights();

    const std::vector<std::shared_ptr<Flight>> getCurrentFlights() { return m_currentFlights; }
    const std::shared_ptr<Flight> getCurrentFlight()
    {
        if (m_currentFlights.size() == 1)
        {
            return m_currentFlights[0];
        }
        return nullptr;
    }
    void setCurrentFlights(std::vector<std::shared_ptr<Flight>> flights)
    {
        m_currentFlights = flights;
    }

    [[nodiscard]] std::shared_ptr<Flight> getFlight(uint64_t flightId) const
    {
        auto it = m_flightIndex.find(flightId);
        if (it != m_flightIndex.end())
        {
            return it->second;
        }
        return nullptr;
    }
    [[nodiscard]] std::vector<std::shared_ptr<Flight>> getFlights() const { return m_flights; }
    void deleteFlight(std::shared_ptr<Flight> flight);

    void setState(const State& state);
    const State& getState() const { return m_latestState; }

    MainWindow* getMainWindow() const { return m_mainWindow; }
    DataStore& getDataStore() { return m_dataStore; }
    std::shared_ptr<NavigraphData> getNavigraph() { return m_navigraph; }

    QString getSetting(const QString &setting);
    void setSetting(const QString &setting, const QString &value);
    QSettings* getSettings() const { return m_settings; }
};

#endif //BLACKBOX_BLACKBOX_H
