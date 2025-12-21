//
// Created by Ian Parker on 21/12/2025.
//

#ifndef BLACKBOX_ROUTEMARKER_H
#define BLACKBOX_ROUTEMARKER_H

#include <QGeoView/Raster/QGVIcon.h>

#include "route.h"

class RouteMarker : public QGVIcon
{
    RoutePoint m_point;

 public:
    RouteMarker(RoutePoint& point);
    ~RouteMarker() override = default;

    QPointF projAnchor() const override
    {
        return projShape().boundingRect().topLeft();
    }
};


#endif //BLACKBOX_ROUTEMARKER_H
