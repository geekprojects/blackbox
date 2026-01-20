//
// Created by Ian Parker on 11/12/2025.
//

#include "flightdetails.h"
#include "../aircrafttypes.h"
#include "../mainwindow.h"

#include <QLabel>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QVBoxLayout>

#include "airportwidget.h"
#include "../blackbox.h"
#include "ui/map/routemap.h"

using namespace std;

FlightDetailsWidget::FlightDetailsWidget(MainWindow* mainWindow, QWidget* parent) : QWidget(parent), m_mainWindow(mainWindow)
{
    auto detailsBoxLayout = new QVBoxLayout();
    detailsBoxLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(detailsBoxLayout);

    auto grid = new QGridLayout();
    grid->setAlignment(Qt::AlignTop);
    int row = 0;
    grid->addWidget(new QLabel("<b>Aircraft:</b>"), row, 0);
    grid->addWidget(m_aircraftTypeLabel = new QLabel(QString::fromStdString(AircraftTypes::getName("B772"))), row, 1);
    row++;

    grid->addWidget(new QLabel("<b>Registration:</b>"), row, 0);
    grid->addWidget(m_registrationLabel = new QLineEdit("DAL39"), row, 1);
    row++;

    grid->addWidget(new QLabel("<b>Call Sign:</b>"), row, 0);
    grid->addWidget(m_callsignLabel = new QLineEdit("DAL39"), row, 1);
    row++;

    grid->addWidget(new QLabel("<b>Origin:</b>"), row, 0);
    grid->addWidget(m_originAirport = new AirportWidget(m_mainWindow->getBlackBoxUI()), row, 1);
    row++;

    grid->addWidget(new QLabel("<b>Destination:</b>"), row, 0);
    grid->addWidget(m_destAirport = new AirportWidget(m_mainWindow->getBlackBoxUI()), row, 1);
    row++;

    QLabel* routeLabel;
    grid->addWidget(routeLabel = new QLabel("<b>Route:</b>"), row, 0);

    m_routeEdit = new QPlainTextEdit("");
    QFontMetrics m (m_routeEdit->font()) ;
    int RowHeight = m.lineSpacing() ;
    m_routeEdit->setFixedHeight  (3 * RowHeight) ;

    auto editBox = new QVBoxLayout();
    editBox->setSpacing(0);
    editBox->addWidget(m_routeEdit);
    connect(m_routeEdit, &QPlainTextEdit::textChanged, this, &FlightDetailsWidget::routeEdited);
    editBox->addWidget(m_updateRouteButton = new QPushButton("Update"));
    m_updateRouteButton->show();
    grid->addLayout(editBox, row, 1);
    grid->setAlignment(routeLabel, Qt::AlignTop);

    detailsBoxLayout->addLayout(grid);
    row++;

    connect(m_registrationLabel, &QLineEdit::editingFinished, this, &FlightDetailsWidget::updated);
    connect(m_callsignLabel, &QLineEdit::editingFinished, this, &FlightDetailsWidget::updated);
    connect(m_originAirport, &AirportWidget::airportChanged, this, &FlightDetailsWidget::updated);
    connect(m_destAirport, &AirportWidget::airportChanged, this, &FlightDetailsWidget::updated);

    connect(m_updateRouteButton, &QPushButton::clicked, this, &FlightDetailsWidget::routeUpdate);

    auto buttonRow = new QHBoxLayout();
    {
        auto deleteButton = new QPushButton(QIcon::fromTheme(QIcon::ThemeIcon::ListRemove), "");
        deleteButton->setSizePolicy(QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Preferred);
        connect(deleteButton, &QPushButton::clicked, m_mainWindow, &MainWindow::deleteCurrentFlight);
        buttonRow->addWidget(deleteButton);
    }
    {
        auto simbriefButton = new QPushButton(QIcon::fromTheme(QIcon::ThemeIcon::SoftwareUpdateAvailable), "");
        simbriefButton->setSizePolicy(QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Preferred);
        buttonRow->addWidget(simbriefButton);
    }
    {
        auto deleteButton = new QPushButton(QIcon::fromTheme(QIcon::ThemeIcon::DocumentProperties), "");
        deleteButton->setSizePolicy(QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Preferred);
        buttonRow->addWidget(deleteButton);
    }
    detailsBoxLayout->addLayout(buttonRow);
}

void FlightDetailsWidget::updateFlight(shared_ptr<Flight> flight)
{
    m_aircraftTypeLabel->setText(QString::fromStdString(AircraftTypes::getName(flight->icaoType)));
    m_registrationLabel->setText(QString::fromStdString(flight->registration));
    m_callsignLabel->setText(QString::fromStdString(flight->flightId));

    m_originAirport->setAirport(flight->origin);
    m_destAirport->setAirport(flight->destination);
    m_routeEdit->setPlainText(QString::fromStdString(flight->route));

    m_updateRouteButton->hide();
}

void FlightDetailsWidget::routeEdited()
{
    auto route = m_routeEdit->toPlainText();
    printf("FlightDetailsWidget::routeChanged: route: %s\n", route.toStdString().c_str());

    auto flight = m_mainWindow->getBlackBoxUI()->getCurrentFlight();
    if (route.toStdString() != flight->route)
    {
        m_updateRouteButton->show();
    }
    else
    {
        m_updateRouteButton->hide();
    }
}

void FlightDetailsWidget::routeUpdate()
{
    auto flight = m_mainWindow->getBlackBoxUI()->getCurrentFlight();
    flight->route = m_routeEdit->toPlainText().toStdString();

    m_mainWindow->getBlackBoxUI()->getDataStore().updateFlight(*flight);

    m_mainWindow->getMap()->refreshRoutes(flight->id);

    m_updateRouteButton->hide();
}

void FlightDetailsWidget::updated()
{
    printf("FlightDetailsWidget::updated: here!\n");
}
