//
// Created by Ian Parker on 25/11/2025.
//

#include "blackbox.h"
#include "mainwindow.h"

#include <QCommandLineParser>

using namespace std;

BlackBoxUI::BlackBoxUI(int argc, char** argv) : m_app(argc, argv)
{
    QApplication::setApplicationName("BlackBox Flight Tracker");
    QApplication::setApplicationVersion("0.1");
    QApplication::setOrganizationName("GeekProjects.com");

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();
    parser.process(m_app);

    m_dataStore.init();

    m_mainWindow = new MainWindow(this);
    m_mainWindow->init();
}

int BlackBoxUI::run()
{
    m_mainWindow->show();
    return m_app.exec();
}

void BlackBoxUI::updateFlights()
{
    const auto flights = m_dataStore.fetchFlights();
    const uint64_t currentFlightId = m_currentFlight.id;

    m_flights.clear();
    uint64_t lastFlightId = 0;
    for (auto flight : flights)
    {
        m_flights.emplace(flight.id, flight);
        if (flight.id > lastFlightId)
        {
            lastFlightId = flight.id;
        }
    }

    const auto it = m_flights.find(currentFlightId);
    if (it == m_flights.end())
    {
        if (!m_flights.empty())
        {
            m_currentFlight = m_flights.at(lastFlightId);
            printf("Updating current flight: %lld\n", m_currentFlight.id);
        }
        else
        {
            m_currentFlight.id = 0;
        }
        m_mainWindow->updateFlights();
    }
}

void BlackBoxUI::setState(const State &state)
{
    m_latestState = state;
    if (m_mainWindow != nullptr)
    {
        m_mainWindow->updateState();
    }
}
