//
// Created by Ian Parker on 02/12/2025.
//

#include "mapoptions.h"

#include <QHBoxLayout>
#include <QPushButton>

MapOptions::MapOptions(std::vector<MapOption> options)
{
    setFixedSize(50, 50);
    setToolButtonStyle(Qt::ToolButtonIconOnly);

    m_menuWidget = new QWidget(this);
    m_menuWidget->setWindowFlags(Qt::Popup | Qt::FramelessWindowHint);
    m_menuWidget->setLayout(new QHBoxLayout());
    m_menuWidget->layout()->setContentsMargins(0, 0, 0, 0);
    m_menuWidget->layout()->setSpacing(0);

    setOptions(options);

    connect(this, &QToolButton::clicked, this, &MapOptions::selectOption);
}

void MapOptions::setOptions(std::vector<MapOption> options)
{
    m_options = options;
    if (!m_options.empty())
    {
        setOption(m_options.front());
    }

    for (auto& option : m_options)
    {
        auto optionMap = new QToolButton();
        optionMap->setFixedSize(50, 50);
        optionMap->setIcon(option.icon);
        optionMap->setText(option.name);
        optionMap->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        connect(optionMap, &QToolButton::clicked, [=]()
        {
            setOption(option);
            selectOption(false);
        });
        m_menuWidget->layout()->addWidget(optionMap);
    }
}

void MapOptions::setOption(const MapOption& option)
{
    setIcon(option.icon);
    //setText(option.name);
}

void MapOptions::selectOption(bool selected)
{
    m_menuWidget->setVisible(!m_menuWidget->isVisible());
    adjustPopup();
}

void MapOptions::adjustPopup()
{
    if (!m_menuWidget->isVisible())
    {
        return;
    }

    QRect rect = geometry();
    QPoint bottomLeft = mapToGlobal(rect.topRight());
    m_menuWidget->move(bottomLeft);
}

void MapOptions::resizeEvent(QResizeEvent* event)
{
    QToolButton::resizeEvent(event);
    adjustPopup();
}

void MapOptions::moveEvent(QMoveEvent* event)
{
    QToolButton::moveEvent(event);
    adjustPopup();
}

