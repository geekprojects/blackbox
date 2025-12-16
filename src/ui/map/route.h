//
// Created by Ian Parker on 16/12/2025.
//

#ifndef BLACKBOX_ROUTE_H
#define BLACKBOX_ROUTE_H

#include <QGeoView/QGVDrawItem.h>

#include "blackbox/datastore.h"

class Route : public QGVDrawItem
{
    std::shared_ptr<Flight> m_flight;

 public:
    Route(std::shared_ptr<Flight> flight);
};


#endif //BLACKBOX_ROUTE_H
