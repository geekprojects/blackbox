//
// Created by Ian Parker on 20/11/2025.
//

#include "mainwindow.h"
#include "map/routemap.h"

#include <QDir>
#include <QNetworkDiskCache>
#include <QTimer>
#include <QVBoxLayout>
#include <QComboBox>
#include <QToolBar>
#include <QMenuBar>
#include <QLabel>
#include <QGroupBox>
#include <qicon.h>
#include <QMessageBox>

#include "liveindicator.h"

using namespace std;

void setupCachedNetworkAccessManager(QObject* parent)
{
    QDir("cacheDir");
    auto cache = new QNetworkDiskCache(parent);
    cache->setCacheDirectory("cacheDir");
    cache->setMaximumCacheSize(100 * 1024 * 1024);
    auto manager = new QNetworkAccessManager(parent);
    manager->setCache(cache);
    QGV::setNetworkManager(manager);
}

MainWindow::MainWindow(BlackBoxUI* bbui) : m_blackBoxUI(bbui)
{
    setWindowTitle("BlackBox Flight Tracker");

    /*
    auto trayIconMenu = new QMenu(this);
    trayIconMenu->addAction(new QAction("Hello!"));
    m_sysTrayIcon = new QSystemTrayIcon(this);
    m_sysTrayIcon->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::DocumentSend));
    m_sysTrayIcon->setContextMenu(trayIconMenu);
    m_sysTrayIcon->show();
    */

    auto menu = menuBar();
    auto fileMenu = menu->addMenu("File");
    fileMenu->addAction("Delete");

    setCentralWidget(new QWidget());
    auto layout = new QVBoxLayout();
    centralWidget()->setLayout(layout);

    // Toolbar
    auto toolbar = addToolBar("Toolbar");
    toolbar->addWidget(m_liveIndicator = new LiveIndicator());

    m_flightComboBox = new QComboBox();
    toolbar->addWidget(m_flightComboBox);

    auto editAction = new QAction("Edit");
    editAction->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::DocumentProperties));
    toolbar->addAction(editAction);

    auto deleteAction = new QAction("Delete");
    deleteAction->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::EditDelete));
    connect(deleteAction, &QAction::triggered, this, &MainWindow::deleteCurrentFlight);
    toolbar->addAction(deleteAction);

    setupCachedNetworkAccessManager(this);

    m_map = new RouteMap(m_blackBoxUI);
    layout->addWidget(m_map);

    auto hbox = new QHBoxLayout();
    hbox->setAlignment(Qt::AlignLeft);
    layout->addLayout(hbox);

    {
        auto infoBox1 = new QGroupBox();
        hbox->addWidget(infoBox1);
        auto infoBox1Layout = new QHBoxLayout();
        infoBox1->setLayout(infoBox1Layout);
        infoBox1->setAlignment(Qt::AlignLeft);
        infoBox1Layout->addWidget(new QLabel("<b>Altitude</b>:"));
        infoBox1Layout->addWidget(m_altitudeLabel = new QLabel(""));
    }
    {
        auto infoBox1 = new QGroupBox();
        hbox->addWidget(infoBox1);
        auto infoBox1Layout = new QHBoxLayout();
        infoBox1->setLayout(infoBox1Layout);
        infoBox1->setAlignment(Qt::AlignLeft);
        infoBox1Layout->addWidget(new QLabel("<b>Speed</b>:"));
        infoBox1Layout->addWidget(m_speedLabel = new QLabel(""));
    }

    connect(m_flightComboBox, &QComboBox::currentIndexChanged, this, [this](int index)
    {
        auto id = m_flightComboBox->itemData(index).toULongLong();
        m_blackBoxUI->setCurrentFlightId(id);
        m_map->resetRoute();
    });

    printf("MainWindow::MainWindow: Done!\n");
}

MainWindow::~MainWindow()
{
}

bool MainWindow::init()
{
    m_blackBoxUI->updateFlights();
    m_map->resetRoute();
    updateState();

    return true;
}


void MainWindow::updateState()
{
    auto state = m_blackBoxUI->getState();

    char buf[1024];
    snprintf(buf, sizeof(buf), "%0.0f feet", state.position.altitude);
    m_altitudeLabel->setText(QString(buf));

    snprintf(buf, sizeof(buf), "%.0f kn", state.groundSpeed);
    m_speedLabel->setText(QString(buf));

    auto now = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count();
    auto diff = now - state.timestamp;
    bool live = diff < 10000; // 10 seconds
    m_liveIndicator->setLive(live);
}

void MainWindow::updateFlights()
{
    map<uint64_t, Flight> flights = m_blackBoxUI->getFlights();
    m_flightComboBox->clear();

    int idx = 0;
    int selectedIndex = 0;
    uint64_t currentFlightId = m_blackBoxUI->getCurrentFlight().id;
    for (auto [flightId, flight] : flights)
    {
        string title = "Flight " + to_string(flight.id) + ": ";

        bool comma = false;
        if (!flight.icaoType.empty())
        {
            title += flight.icaoType;
            comma = true;
        }
        if (!flight.flightId.empty() && flight.icaoType != flight.flightId)
        {
            if (comma)
            {
                title += ", ";
            }
            title += flight.flightId;
            comma = true;
        }

        if (!flight.origin.empty())
        {
            if (comma)
            {
                title += ", ";
            }
            title += "Origin: " + flight.origin;
            comma = true;
        }
        if (!flight.destination.empty())
        {
            if (comma)
            {
                title += ", ";
            }
            title += "Destination: " + flight.destination;
        }
        m_flightComboBox->addItem(QString(title.c_str()), QVariant::fromValue(flight.id));

        if (flight.id == currentFlightId)
        {
            printf("updateFlights: Found current Flight: %lld -> %d\n", flight.id, idx);
            selectedIndex = idx;
        }

        idx++;
    }
    m_flightComboBox->setCurrentIndex(selectedIndex);
}

void MainWindow::deleteCurrentFlight()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(
        this,
        "Delete Flight",
        "Are you sure you wish to delete this fight? This action cannot be undone.",
        QMessageBox::Yes|QMessageBox::No);
    if (reply == QMessageBox::Yes)
    {
        // Well, we'd better delete it, then
        m_blackBoxUI->getDataStore().deleteFlight(m_blackBoxUI->getCurrentFlight().id);
        m_blackBoxUI->updateFlights();
    }
}
