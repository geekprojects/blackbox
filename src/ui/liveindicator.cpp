//
// Created by Ian Parker on 26/11/2025.
//

#include "liveindicator.h"

#include <QPainter>

LiveIndicator::LiveIndicator(QWidget* parent) : QWidget(parent)
{
    setFixedSize(20, 20);
}

void LiveIndicator::paintEvent(QPaintEvent* paint_event)
{
    QWidget::paintEvent(paint_event);

    QPainter p(this);

    p.setBrush(QBrush(Qt::white, Qt::SolidPattern));
    p.drawEllipse(1,1,18,18);

    if (m_live)
    {
        p.setBrush(QBrush(Qt::green, Qt::SolidPattern));
    }
    else
    {
        p.setBrush(QBrush(Qt::gray, Qt::SolidPattern));
    }
    p.drawEllipse(2,2,16,16);
}
