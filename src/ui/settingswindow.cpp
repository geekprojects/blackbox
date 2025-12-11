//
// Created by Ian Parker on 11/12/2025.
//

#include "settingswindow.h"

#include <QLabel>
#include <QTabWidget.h>
#include <QVBoxLayout>

SettingsWindow::SettingsWindow()
{
    setWindowTitle("Settings");

    auto mainLayout = new QVBoxLayout();
    setLayout(mainLayout);

    auto tabs = new QTabWidget();

    auto simTab = new QWidget();
    simTab->setLayout(new QVBoxLayout());
    simTab->layout()->addWidget(new QLabel("Simulator stuff"));
    tabs->addTab(simTab, "Simulator");

    auto mapTab = new QWidget();
    mapTab->setLayout(new QVBoxLayout());
    mapTab->layout()->addWidget(new QLabel("Map"));
    tabs->addTab(mapTab, "Map");

    mainLayout->addWidget(tabs);
}
