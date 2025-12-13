//
// Created by Ian Parker on 11/12/2025.
//

#ifndef BLACKBOX_FLIGHTDETAILS_H
#define BLACKBOX_FLIGHTDETAILS_H

#include <QLabel>
#include <QPlainTextEdit>

#include "blackbox/datastore.h"

class QPushButton;
class MainWindow;

class FlightDetailsWidget : public QWidget
{
    MainWindow* m_mainWindow;

    QLabel* m_aircraftTypeLabel;
    QLabel* m_registrationLabel;
    QLabel* m_callsignLabel;
    QLabel* m_originLabel;
    QLabel* m_destLabel;
    QPlainTextEdit* m_routeEdit;

    QPushButton* m_updateRouteButton;

    void routeChanged();

 public:
    explicit FlightDetailsWidget(MainWindow* mainWindow, QWidget* parent = nullptr);

    void updateFlight(Flight& flight);
};


#endif //BLACKBOX_FLIGHTDETAILS_H
