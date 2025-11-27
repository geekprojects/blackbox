//
// Created by Ian Parker on 26/11/2025.
//

#include "landingicon.h"

LandingIcon::LandingIcon(const State &landingState) : m_landingState(landingState)
{
    QImage planeIcon("../data/images/landing.png");
    loadImage(planeIcon);

    setFlag(QGV::ItemFlag::Clickable, true);
}

bool LandingIcon::event(QEvent* event)
{
    printf("LandingIcon::event: HERE!\n");
    return QGVIcon::event(event);
}

void LandingIcon::projOnMouseClick(const QPointF &projPos)
{
    QGVIcon::projOnMouseClick(projPos);
    printf("LandingIcon::projOnMouseClick: FPM=%0.2f\n", m_landingState.fpm);
}

QString LandingIcon::projTooltip(const QPointF &projPos) const
{
    char buf[1024];
    snprintf(buf, sizeof(buf), "FPM: %0.2f", m_landingState.fpm);

const char* rating;
    if (m_landingState.fpm >= -125.0)
    {
        rating = "Butter!";
    }


    return QString(buf);
}
