//
// Created by Ian Parker on 01/12/2025.
//

#include "mapmenu.h"

#include <QLabel>
#include <QPushButton>
#include <QToolButton>
#include <QVBoxLayout>

#include "mapoptions.h"

using namespace std;

MapMenu::MapMenu()
{
    setAutoFillBackground(true);

    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    setLayout(layout);
    layout->setAlignment(Qt::AlignTop);
    //layout->addWidget(new QPushButton("Hello!"));
    //layout->addWidget(new QPushButton("World!"));

    QIcon planeIcon("../data/images/tower-control.svg");

    vector<MapOption> options = {
        {.name = "Map", .icon = planeIcon},
        {.name = "Track", .icon = planeIcon},
    };
    auto mapButton = new MapOptions(options);
    layout->addWidget(mapButton);

    vector<MapOption> options2 = {
        {.name = "Altitude", .icon = planeIcon},
        {.name = "G Forces", .icon = planeIcon},
    };
    auto lineButton = new MapOptions(options2);
    layout->addWidget(lineButton);

/*
    {
        m_mapButton = new QToolButton();//planeIcon, "Map");
        m_mapButton->setFixedSize(50, 50);
        m_mapButton->setIcon(planeIcon);
        m_mapButton->setText("Map");
        m_mapButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        layout->addWidget(m_mapButton);

        m_menuWidget = new QWidget(this);
        m_menuWidget->setWindowFlags(Qt::Popup | Qt::FramelessWindowHint);
        m_menuWidget->setLayout(new QHBoxLayout());
        m_menuWidget->layout()->setContentsMargins(0, 0, 0, 0);
        m_menuWidget->layout()->setSpacing(0);
        {
            auto optionMap = new QToolButton();
            optionMap->setFixedSize(50, 50);
            optionMap->setIcon(planeIcon);
            optionMap->setText("Track");
            optionMap->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
            m_menuWidget->layout()->addWidget(optionMap);
        }
        {
            auto optionMap = new QToolButton();//planeIcon, "Map");
            optionMap->setFixedSize(50, 50);
            optionMap->setIcon(planeIcon);
            optionMap->setText("Flights");
            optionMap->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
            m_menuWidget->layout()->addWidget(optionMap);
        }
        connect(m_mapButton,&QPushButton::clicked,[=]()
        {
        });

    }
    {
        auto mapButton = new QToolButton();//planeIcon, "Map");
        mapButton->setFixedSize(50, 50);
        mapButton->setIcon(planeIcon);
        mapButton->setText("Map");
        mapButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        //mapButton->setStyleSheet("QToolButton {background-color: #A3C1DA; color: red;}");
        layout->addWidget(mapButton);
    }
*/
}

MapMenu::~MapMenu()
{
}
