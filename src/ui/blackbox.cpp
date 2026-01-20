//
// Created by Ian Parker on 25/11/2025.
//

#include "blackbox.h"
#include "mainwindow.h"
#include "flightswindow.h"

#include "map/route.h"

#include <QCommandLineParser>
#include <QSettings>
#include <QFileDialog>
#include <QMessageBox>
#include <QNetworkDiskCache>
#include <QStandardPaths>

#include "flightswindow.h"
#include "map/route.h"

using namespace std;

BlackBoxUI::BlackBoxUI(int argc, char** argv) : QApplication(argc, argv)
{
    setApplicationName("BlackBox Flight Tracker");
    setApplicationVersion("0.1");
    setOrganizationName("GeekProjects.com");

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();
    parser.process(*this);

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

    string homeDir = getenv("HOME");
    m_navigraph = make_shared<NavigraphData>(homeDir + "/.config/ABarthel/little_navmap_db/little_navmap_navigraph.sqlite");
    m_navigraph->open();

    setupCachedNetworkAccessManager();

    m_mainWindow = new MainWindow(this);
    m_flightsWindow = new FlightsWindow(this);

    m_mainWindow->init();
}

int BlackBoxUI::run()
{
    /*
    auto flight = make_shared<Flight>();
    flight->origin = "SBGL";
    flight->destination = "SAEZ";
    flight->route = "SBGL/10 TIVR1A BITAK UN857 NELOX UM534 DADUT UN741 PAPIX DCT SAEZ/29";

    Route route(m_navigraph, flight);

    route.parseRoute();
*/
    m_mainWindow->show();
    return exec();
}

void BlackBoxUI::openFlightsWindow() const
{
    m_flightsWindow->show();
}

void BlackBoxUI::updateFlights()
{
    auto flights = m_dataStore.fetchFlights();

    m_flights.clear();
    for (auto flight : flights)
    {
        shared_ptr<Flight> flightPtr = make_shared<Flight>(flight);
        m_flights.push_back(flightPtr);
    }

    uint64_t lastFlightTimestamp = 0;
    shared_ptr<Flight> lastFlight = nullptr;
    for (auto flight : m_flights)
    {
        m_flightIndex.emplace(flight->id, flight);
        if (flight->startTime > lastFlightTimestamp)
        {
            lastFlightTimestamp = flight->startTime;
            lastFlight = flight;
        }
        else if (lastFlight == nullptr)
        {
            lastFlight = flight;
        }

        flight->stateCount = m_dataStore.countUpdates(flight->id);
    }

    bool foundCurrentFlight = false;
    if (m_currentFlight != nullptr)
    {
        const auto it = m_flightIndex.find(m_currentFlight->id);
        foundCurrentFlight = it != m_flightIndex.end();
    }

    if (!foundCurrentFlight)
    {
        if (!m_flights.empty())
        {
            m_currentFlight = lastFlight;
            printf("Updating current flight: %lld\n", m_currentFlight->id);
        }
        else
        {
            m_currentFlight = nullptr;
        }
    }
    m_mainWindow->updateFlights();
    m_flightsWindow->updateFlights();
}

void BlackBoxUI::deleteFlight(std::shared_ptr<Flight> flight)
{
    auto reply = QMessageBox::question(
        m_mainWindow,
        "Delete Flight",
        QString::fromStdString("Are you sure you wish to delete the fight '" + flight->toString() + "'? This action cannot be undone."),
        QMessageBox::Yes|QMessageBox::No);
    if (reply == QMessageBox::Yes)
    {
        // Well, we'd better delete it, then
        getDataStore().deleteFlight(flight->id);
        updateFlights();
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

void BlackBoxUI::setupCachedNetworkAccessManager()
{
    QDir("cacheDir");
    auto cache = new QNetworkDiskCache(this);

    auto cacheLocations = QStandardPaths::standardLocations(QStandardPaths::CacheLocation);
    auto cacheDir = cacheLocations.first() + "/mapcache";
    printf("Setting map cache dir: %s\n", cacheDir.toStdString().c_str());
    cache->setCacheDirectory(cacheDir);

    cache->setMaximumCacheSize(100 * 1024 * 1024);
    auto manager = new QNetworkAccessManager(this);
    manager->setCache(cache);
    QGV::setNetworkManager(manager);
}

