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
#include <QPushButton>
#include <QStandardPaths>
#include <QToolBox>
#include <QToolButton>

#include "aircrafttypes.h"
#include "widgets/flightdetails.h"
#include "widgets/liveindicator.h"

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

    auto trayIconMenu = new QMenu(this);

    QAction* showAction;
    trayIconMenu->addAction(showAction = new QAction("Show"));
    connect(showAction, &QAction::triggered, this, &MainWindow::showNormal);

    QAction* quitAction;
    trayIconMenu->addAction(quitAction = new QAction("Quit"));
    connect(quitAction, &QAction::triggered, qApp, &QApplication::quit);

    m_sysTrayIcon = new QSystemTrayIcon(this);
    m_sysTrayIcon->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::DocumentSend));
    m_sysTrayIcon->setContextMenu(trayIconMenu);
    m_sysTrayIcon->show();

    m_settingsWindow = new SettingsWindow();

    auto menu = menuBar();
    auto fileMenu = menu->addMenu("File");
    fileMenu->addAction("Delete");
    fileMenu->addAction("Settings", [this]
    {
        m_settingsWindow->show();
    });
    auto helpMenu = menu->addMenu("Help");
    helpMenu->addAction("About", [this]
    {
        QMessageBox::about(this, "BlackBox Flight Tracker", "BlackBox Flight Tracker v0.1");
    });
    helpMenu->addAction("About Qt", [this]
    {
        QMessageBox::aboutQt(this);
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
            selectFlight(id);
        }
    });

    QGVWidget* settingsWidget = new QGVWidget();
    settingsWidget->setAnchor(QPoint(5, 5), {Qt::RightEdge, Qt::TopEdge });
    QToolButton* settingsButton = new QToolButton(this);
    settingsButton->setIcon(QIcon(":/images/cog.svg"));
    settingsButton->setIconSize(QSize(20, 20));
    settingsWidget->setLayout(new QVBoxLayout());
    settingsWidget->layout()->addWidget(settingsButton);
    m_map->addWidget(settingsWidget);

    {
        QMenu* mapSettingsMenu = new QMenu();
        mapSettingsMenu->addSection("Trail Type:");
        QAction* altitudeAction = mapSettingsMenu->addAction(QIcon(":/images/plane-marker.svg"), "Altitude");
        altitudeAction->setCheckable(true);
        altitudeAction->setChecked(true);
        auto gforceAction = mapSettingsMenu->addAction(QIcon(":/images/trending-up-down.svg"), "G-Forces");
        gforceAction->setCheckable(true);
        settingsButton->setPopupMode(QToolButton::InstantPopup);
        settingsButton->setMenu(mapSettingsMenu);
    }

    printf("MainWindow::MainWindow: Done!\n");
}

MainWindow::~MainWindow() = default;

void MainWindow::closeEvent(QCloseEvent* event)
{
    if (!event->spontaneous() || !isVisible())
    {
        return;
    }

    if (m_sysTrayIcon->isVisible())
    {
        hide();
        event->ignore();
    }
}

void MainWindow::buildFlightSelectionWidget()
{
    auto toolbarLayout = new QVBoxLayout();
    toolbarLayout->setContentsMargins(0, 0, 0, 0);
    toolbarLayout->setSpacing(0);

    m_flightBox = new QGroupBox();
    toolbarLayout->addWidget(m_flightBox);

    auto toolbarInner = new QVBoxLayout();
    m_flightBox->setLayout(toolbarInner);
    m_flightBox->setStyleSheet("QGroupBox {background-color: rgba(100, 100, 100, 220); border-radius: 10px; }");

    auto topRow = new QHBoxLayout();
    auto globeButton = new QToolButton();
    globeButton->setIcon(QIcon(":/images/map-pin.svg"));
    globeButton->setPopupMode(QToolButton::InstantPopup);

    QMenu* mapModeMenu = new QMenu();
    mapModeMenu->addAction(QIcon(":/images/map-pin.svg"), "Flight");
    mapModeMenu->addAction(QIcon(":/images/globe.svg"), "Global");
    globeButton->setMenu(mapModeMenu);

    topRow->addWidget(globeButton);

    m_flightComboBox = new QComboBox();
    m_flightComboBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    m_flightComboBox->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Preferred);
    m_flightComboBox->addItem("MMMMMMMMMMMMMMMMM");
    topRow->addWidget(m_flightComboBox);

    auto expandButton = new QToolButton();
    expandButton->setStyleSheet("QToolButton {border: 0;}");
    expandButton->setArrowType(Qt::ArrowType::UpArrow);
    connect(expandButton, &QPushButton::clicked, this, [this, expandButton]
    {
        bool hidden = m_detailsBox->isHidden();
        if (hidden)
        {
            m_detailsBox->show();
            expandButton->setArrowType(Qt::ArrowType::UpArrow);
        }
        else
        {
            m_detailsBox->hide();
            expandButton->setArrowType(Qt::ArrowType::DownArrow);
        }
        m_flightBox->adjustSize();
        m_map->removeWidget(m_flightWidget);
        m_map->addWidget(m_flightWidget);
    });
    topRow->addWidget(expandButton);
    toolbarInner->addLayout(topRow);

    m_detailsBox = new FlightDetailsWidget(this);
    toolbarInner->addWidget(m_detailsBox);

    m_flightWidget = new QGVWidget();
    m_flightWidget->setLayout(toolbarLayout);
    m_flightWidget->setAnchor(QPoint(5, 5), {Qt::LeftEdge, Qt::TopEdge });
    m_map->addWidget(m_flightWidget);
}

void MainWindow::buildInfoWidget()
{
    m_infoWidget = new QGVWidget();
    auto hbox = new QHBoxLayout();
    m_infoWidget->setLayout(hbox);
    m_infoWidget->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Preferred);
    hbox->setAlignment(Qt::AlignCenter);
    m_infoWidget->setAnchor(QPoint(0, 0), {Qt::BottomEdge });

    hbox->addWidget(m_liveIndicator = new LiveIndicator());
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
    m_sysTrayIcon->showMessage("Landing", "Butter!");
    map<uint64_t, Flight> flights = m_blackBoxUI->getFlights();
    m_flightComboBox->clear();

    int idx = 0;
    int selectedIndex = 0;
    uint64_t currentFlightId = m_blackBoxUI->getCurrentFlight().id;
    for (auto [flightId, flight] : flights)
    {
        string title;

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
                title += ": ";
            }
            title += flight.flightId;
            comma = true;
        }

        bool hasOrigin = false;
        if (!flight.origin.empty())
        {
            if (comma)
            {
                title += ": ";
            }
            title += flight.origin;
            comma = true;
            hasOrigin = true;
        }
        if (!flight.destination.empty())
        {
            if (hasOrigin)
            {
                title += " - ";
            }
            else if (comma)
            {
                title += ": ";
            }
            title += flight.destination;
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

void MainWindow::selectFlight(uint64_t flightId)
{
    m_blackBoxUI->setCurrentFlightId(flightId);
    m_map->showFlight(flightId);

    m_detailsBox->updateFlight(m_blackBoxUI->getCurrentFlight());
}
