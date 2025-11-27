//
// Created by Ian Parker on 26/11/2025.
//

#ifndef BLACKBOX_LANDINGICON_H
#define BLACKBOX_LANDINGICON_H

#include <QGeoView/Raster/QGVIcon.h>

#include "blackbox/state.h"

class LandingIcon : public QGVIcon
{
    Q_OBJECT

    State m_landingState;

 public:
    LandingIcon(const State &landingState);
    ~LandingIcon() override = default;

    bool event(QEvent* event) override;

    void projOnMouseClick(const QPointF &projPos) override;

    QString projTooltip(const QPointF &projPos) const override;
};


#endif //BLACKBOX_LANDINGICON_H
