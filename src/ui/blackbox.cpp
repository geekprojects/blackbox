//
// Created by Ian Parker on 25/11/2025.
//

#include "blackbox.h"
#include "mainwindow.h"

#include <QCommandLineParser>
#include <QSettings>
#include <QFileDialog>

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


    QSettings settings("geekprojects", "BlackBox");
    printf("Settings file: %s\n", settings.fileName().toStdString().c_str());

    string xplaneDir;
    auto xplaneDirValue = settings.value("XPlaneDir");
    if (xplaneDirValue.isValid())
    {
        xplaneDir = xplaneDirValue.toString().toStdString();
    }
    else
    {
        printf("No XPlane directory set, please set in settings\n");
        auto dialog = QFileDialog::getExistingDirectory(nullptr, "Select XPlane directory");
        printf("dir=%s\n", dialog.toStdString().c_str());
        if (dialog.isEmpty())
        {
            printf("No XPlane directory specified, exiting\n");
            exit(1);
        }

        std::filesystem::path xpPath(dialog.toStdString());
        if (!exists(xpPath) && !is_directory(xpPath))
        {
            printf("No XPlane directory is not valid, exiting\n");
            exit(1);
        }

        std::filesystem::path resourcesPath = xpPath / "Resources" / "plugins";
        if (!exists(xpPath) && !is_directory(xpPath))
        {
            printf("No plugins directory!");
            exit(1);
        }

        xplaneDir = dialog.toStdString();
        settings.setValue("XPlaneDir", QVariant::fromValue(QString::fromStdString(xplaneDir)));
        settings.sync();
    }

    printf("XPlane directory: %s\n", xplaneDir.c_str());
    auto databasePath = filesystem::path(xplaneDir) / "Output" / "blackbox";
    if (!exists(databasePath))
    {
        create_directory(databasePath);
    }

    printf("Database directory: %s\n", databasePath.c_str());
    auto databaseFile = databasePath / "blackbox.db";

    m_dataStore.init(databaseFile.string());

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
