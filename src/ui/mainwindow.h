//
// Created by Ian Parker on 20/11/2025.
//

#ifndef BLACKBOX_MAINWINDOW_H
#define BLACKBOX_MAINWINDOW_H

#include <QBoxLayout>
#include <QComboBox>
#include <qgroupbox.h>
#include <QLabel>
#include <QMainWindow>
#include <QSystemTrayIcon>

#include <QGeoView/QGVWidget.h>

#include "blackbox.h"
#include "settingswindow.h"
#include "blackbox/datastore.h"

class FlightDetailsWidget;
class LiveIndicator;
class RouteMap;

class MainWindow : public QMainWindow
{
    Q_OBJECT

    BlackBoxUI* m_blackBoxUI = nullptr;

    QSystemTrayIcon* m_sysTrayIcon = nullptr;

    LiveIndicator* m_liveIndicator = nullptr;
    QComboBox* m_flightComboBox = nullptr;

    QGVWidget* m_flightWidget = nullptr;
    QGroupBox* m_flightBox = nullptr;
    FlightDetailsWidget* m_detailsBox = nullptr;

    QGVWidget* m_infoWidget = nullptr;
    QLabel* m_altitudeLabel = nullptr;
    QLabel* m_speedLabel = nullptr;
    QLabel* m_headingLabel = nullptr;

    RouteMap* m_map = nullptr;

    SettingsWindow* m_settingsWindow = nullptr;

    void importVolanta();

public:
    explicit MainWindow(BlackBoxUI* blackBoxUI);
    ~MainWindow() override;

    void closeEvent(QCloseEvent* event) override;

    void selectFlight(uint64_t flightId);
    void updateFlights();

    void buildFlightSelectionWidget();

    QLabel* createInfoBox(QHBoxLayout* hbox, std::string label);
    void buildInfoWidget();

    bool init();

    void updateState();

    void deleteCurrentFlight();

    [[nodiscard]] BlackBoxUI* getBlackBoxUI() const { return m_blackBoxUI; }
};

#endif //BLACKBOX_MAINWINDOW_H
