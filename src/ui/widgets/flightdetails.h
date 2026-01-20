//
// Created by Ian Parker on 11/12/2025.
//

#ifndef BLACKBOX_FLIGHTDETAILS_H
#define BLACKBOX_FLIGHTDETAILS_H

#include <QLabel>
#include <QPlainTextEdit>
#include <QLineEdit>

#include "blackbox/datastore.h"

class AirportWidget;
class QPushButton;
class MainWindow;

class FlightDetailsWidget : public QWidget
{
    MainWindow* m_mainWindow;

    QLabel* m_aircraftTypeLabel;
    QLineEdit* m_registrationLabel;
    QLineEdit* m_callsignLabel;
    AirportWidget* m_originAirport;
    AirportWidget* m_destAirport;
    QPlainTextEdit* m_routeEdit;

    QPushButton* m_updateRouteButton;

    void routeEdited();
    void routeUpdate();
    void updated();

 public:
    explicit FlightDetailsWidget(MainWindow* mainWindow, QWidget* parent = nullptr);

    void updateFlight(std::shared_ptr<Flight> flight);
};


#endif //BLACKBOX_FLIGHTDETAILS_H
