//
// Created by Ian Parker on 20/11/2025.
//

#include "mainwindow.h"
#include "map/routemap.h"
#include "../common/utils.h"

#include <QDir>
#include <QNetworkDiskCache>
#include <QTimer>
#include <QVBoxLayout>
#include <QComboBox>
#include <QToolBar>
#include <QMenuBar>
#include <QLabel>
#include <QGroupBox>
#include <QMessageBox>
#include <QStandardPaths>

#include "liveindicator.h"

using namespace std;

void setupCachedNetworkAccessManager(QObject* parent)
{
    QDir("cacheDir");
    auto cache = new QNetworkDiskCache(parent);

    auto cacheLocations = QStandardPaths::standardLocations(QStandardPaths::CacheLocation);
    auto cacheDir = cacheLocations.first() + "/mapcache";
    printf("Setting map cache dir: %s\n", cacheDir.toStdString().c_str());
    cache->setCacheDirectory(cacheDir);

    cache->setMaximumCacheSize(100 * 1024 * 1024);
    auto manager = new QNetworkAccessManager(parent);
    manager->setCache(cache);
    QGV::setNetworkManager(manager);
}

MainWindow::MainWindow(BlackBoxUI* bbui) : m_blackBoxUI(bbui)
{
    setWindowTitle("BlackBox Flight Tracker");

    setupCachedNetworkAccessManager(this);

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
    auto helpMenu = menu->addMenu("Help");
    helpMenu->addAction("About", [this]
    {
        QMessageBox::about(this, "BlackBox Flight Tracker", "BlackBox Flight Tracker v0.1");
    });

    setCentralWidget(new QWidget());

    auto mainLayout = new QVBoxLayout();
    mainLayout->setContentsMargins(0, 0, 0, 0);
    centralWidget()->setLayout(mainLayout);

    m_map = new RouteMap(m_blackBoxUI);
    mainLayout->addWidget(m_map);

    buildFlightSelectionWidget();
    buildInfoWidget();

    connect(m_flightComboBox, &QComboBox::currentIndexChanged, this, [this](int index)
    {
        if (index >= 0)
        {
            auto id = m_flightComboBox->itemData(index).toULongLong();
            m_blackBoxUI->setCurrentFlightId(id);
            m_map->showFlight(id);
        }
    });

    printf("MainWindow::MainWindow: Done!\n");
}

MainWindow::~MainWindow()
{
}

void MainWindow::buildFlightSelectionWidget()
{
    auto toolbar = new QHBoxLayout();
    toolbar->setContentsMargins(0, 0, 0, 0);
    toolbar->setSpacing(0);

    auto toolbarBox = new QGroupBox();
    toolbar->addWidget(toolbarBox);

    auto toolbarInner = new QHBoxLayout();
    toolbarBox->setLayout(toolbarInner);
    toolbarBox->setStyleSheet("QGroupBox {background-color: rgba(100, 100, 100, 128); border-radius: 10px; }");

    toolbarInner->addWidget(m_liveIndicator = new LiveIndicator());

    m_flightComboBox = new QComboBox();
    m_flightComboBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    m_flightComboBox->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Preferred);
    m_flightComboBox->addItem("MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM");
    toolbarInner->addWidget(m_flightComboBox);

    auto flightWidget = new QGVWidget();
    flightWidget->setLayout(toolbar);
    flightWidget->setAnchor(QPoint(0, 5), {Qt::TopEdge });
    m_map->addWidget(flightWidget);
}

void MainWindow::buildInfoWidget()
{
    m_infoWidget = new QGVWidget();
    auto hbox = new QHBoxLayout();
    m_infoWidget->setLayout(hbox);
    m_infoWidget->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Preferred);
    hbox->setAlignment(Qt::AlignCenter);
    m_infoWidget->setAnchor(QPoint(0, 0), {Qt::BottomEdge });

    m_altitudeLabel = createInfoBox(hbox, "Altitude");
    m_speedLabel = createInfoBox(hbox, "Speed");
    m_headingLabel = createInfoBox(hbox, "Heading");
    m_map->addWidget(m_infoWidget);
}

QLabel* MainWindow::createInfoBox(QHBoxLayout* hbox, std::string label)
{
    auto infoBox1 = new QGroupBox();
    //infoBox1->setStyleSheet("QGroupBox { background-color: green; });");
    infoBox1->setStyleSheet("QGroupBox {background-color: rgba(100, 100, 100, 128); border-radius: 10px; }");
    hbox->addWidget(infoBox1);
    auto infoBox1Layout = new QHBoxLayout();
    infoBox1->setLayout(infoBox1Layout);
    infoBox1->setAlignment(Qt::AlignLeft);
    infoBox1->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Preferred);
    infoBox1Layout->addWidget(new QLabel(QString::fromStdString("<b>" + label + "</b>:")));

    QLabel* labelPtr = new QLabel("99999 units");
    infoBox1Layout->addWidget(labelPtr);
    return labelPtr;
}

bool MainWindow::init()
{
    m_blackBoxUI->updateFlights();
    updateState();

    m_map->setMode(MapMode::ROUTE);

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

    snprintf(buf, sizeof(buf), "%.0f°", state.yaw);
    m_headingLabel->setText(QString(buf));

    m_map->resize(m_map->size());

    auto now = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count();
    auto diff = now - state.timestamp;
    bool live = diff < 10000; // 10 seconds
    m_liveIndicator->setLive(live);

    m_infoWidget->resize(m_infoWidget->size());
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
        m_map->clearRoutes();
        m_blackBoxUI->getDataStore().deleteFlight(m_blackBoxUI->getCurrentFlight().id);
        m_blackBoxUI->updateFlights();
    }
}
