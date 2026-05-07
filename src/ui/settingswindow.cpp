//
// Created by Ian Parker on 11/12/2025.
//

#include "settingswindow.h"

#include <QComboBox>
#include <QFileDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSettings>
#include <QTabWidget.h>
#include <QVBoxLayout>

#include "blackbox.h"

SettingsWindow::SettingsWindow(BlackBoxUI* blackBoxUI) : m_blackBoxUI(blackBoxUI)
{
    setWindowTitle("Settings");

    auto mainLayout = new QVBoxLayout();
    setLayout(mainLayout);

    auto tabs = new QTabWidget();

    {
        auto simTab = new QWidget();
        auto grid = new QGridLayout();
        simTab->setLayout(grid);
        grid->addWidget(new QLabel("Simulator:"), 0, 0);
        QComboBox* simComboBox = new QComboBox();
        grid->addWidget(simComboBox, 0, 1);
        simComboBox->addItem("X-Plane 12");

        grid->addWidget(new QLabel("Path:"), 1, 0);

        auto pathBox = new QHBoxLayout();
        pathBox->addWidget(m_simPathEdit = new QLineEdit());
        QPushButton* pathButton = new QPushButton("Browse");
        pathBox->addWidget(pathButton);
        grid->addLayout(pathBox, 1, 1);

        connect(pathButton, &QPushButton::clicked, [this]()
        {
            QString dir = m_blackBoxUI->getSetting("XPlaneDir");
            if (dir.isEmpty())
            {
                dir = QDir::homePath();
            }
            QString directory = QFileDialog::getExistingDirectory(
                this,
                tr("X-Plane Directory"),
                dir);
            if (!directory.isEmpty())
            {
                m_blackBoxUI->setSetting("XPlaneDir", directory);
                update();
            }
        });

        tabs->addTab(simTab, "Simulator");
    }

    {
        auto simbriefTab = new QWidget();
        auto grid = new QGridLayout();
        simbriefTab->setLayout(grid);
        grid->addWidget(new QLabel("Username:"), 0, 0);
        grid->addWidget(m_simbriefUsernameEdit = new QLineEdit(), 0, 1);
        tabs->addTab(simbriefTab, "Simbrief");

        connect(m_simbriefUsernameEdit, &QLineEdit::textChanged, [this]()
        {
            m_blackBoxUI->setSetting("SimbriefUsername", m_simbriefUsernameEdit->text());
            update();
        });
    }

    auto mapTab = new QWidget();
    mapTab->setLayout(new QVBoxLayout());
    mapTab->layout()->addWidget(new QLabel("Map"));
    tabs->addTab(mapTab, "Map");

    mainLayout->addWidget(tabs);

    update();
}

void SettingsWindow::update()
{
    m_simPathEdit->setText(m_blackBoxUI->getSetting("XPlaneDir"));
    m_simbriefUsernameEdit->setText(m_blackBoxUI->getSetting("SimbriefUsername"));
}
