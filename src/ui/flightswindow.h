//
// Created by Ian Parker on 03/01/2026.
//

#ifndef BLACKBOX_FLIGHTSWINDOW_H
#define BLACKBOX_FLIGHTSWINDOW_H

#include <QTableView>
#include <QWidget>
#include <QTableWidget>

#include "blackbox.h"


class FlightsWindow : public QWidget
{
    BlackBoxUI* m_blackBoxUI = nullptr;

    QTableWidget* m_tableWidget = nullptr;
    
    std::map<int, std::shared_ptr<Flight>> m_flightIndex;

 public:
    explicit FlightsWindow(BlackBoxUI* blackBoxUI);
    ~FlightsWindow() override = default;

    void updateFlights();

    void mergeSelected();
};


#endif //BLACKBOX_FLIGHTSWINDOW_H
