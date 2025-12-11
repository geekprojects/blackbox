//
// Created by Ian Parker on 11/12/2025.
//

#include "flightdetails.h"
#include "../aircrafttypes.h"
#include "../mainwindow.h"

#include <QLabel>
#include <QPlainTextEdit>
#include <QPushButton>
#include <qtextedit.h>
#include <QVBoxLayout>

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

    grid->addWidget(new QLabel("<b>Call Sign:</b>"), row, 0);
    grid->addWidget(m_callsignLabel = new QLabel("DAL39"), row, 1);
    row++;

    grid->addWidget(new QLabel("<b>Origin:</b>"), row, 0);
    grid->addWidget(m_originLabel = new QLabel("Gatwick"), row, 1);
    row++;

    grid->addWidget(new QLabel("<b>Destination:</b>"), row, 0);
    grid->addWidget(m_destLabel = new QLabel("Miami"), row, 1);
    row++;

    grid->addWidget(new QLabel("<b>Route:</b>"), row, 0);

    m_routeEdit = new QPlainTextEdit("CPT3G CPT L9 DIDZA N14 OKTAD MEDOG VATRY RESNO NATF LOMSI ANATI CATOG SSENA ART JHW EWC AIR HVQ HLRRY ONDRE1");
    QFontMetrics m (m_routeEdit->font()) ;
    int RowHeight = m.lineSpacing() ;
    m_routeEdit->setFixedHeight  (3 * RowHeight) ;
    grid->addWidget(m_routeEdit, row, 1);
    detailsBoxLayout->addLayout(grid);
    row++;

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

void FlightDetailsWidget::updateFlight(Flight &flight)
{
    m_aircraftTypeLabel->setText(QString::fromStdString(AircraftTypes::getName(flight.icaoType)));
    m_callsignLabel->setText(QString::fromStdString(flight.flightId));
    m_originLabel->setText(QString::fromStdString(flight.origin));
    m_destLabel->setText(QString::fromStdString(flight.destination));
}
