//
// Created by Ian Parker on 20/11/2025.
//

#ifndef BLACKBOX_MAINWINDOW_H
#define BLACKBOX_MAINWINDOW_H

#include <QComboBox>
#include <QLabel>
#include <QMainWindow>
#include <QSystemTrayIcon>

#include "blackbox.h"
#include "blackbox/datastore.h"

class LiveIndicator;
class RouteMap;

class MainWindow : public QMainWindow
{
    Q_OBJECT

    BlackBoxUI* m_blackBoxUI;

    QSystemTrayIcon* m_sysTrayIcon;

    LiveIndicator* m_liveIndicator;
    QComboBox* m_flightComboBox = nullptr;

    QLabel* m_altitudeLabel = nullptr;
    QLabel* m_speedLabel = nullptr;

    RouteMap* m_map;

    void deleteCurrentFlight();

public:
    void updateFlights();

    explicit MainWindow(BlackBoxUI* blackBoxUI);
    ~MainWindow() override;

    bool init();

    void updateState();
};

#endif //BLACKBOX_MAINWINDOW_H
