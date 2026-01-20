//
// Created by Ian Parker on 03/01/2026.
//

#include "flightswindow.h"

#include <qdatetime.h>
#include <QPushButton>
#include <QVBoxLayout>

#include "aircrafttypes.h"
#include "mainwindow.h"
#include "utils/mergeflights.h"

using namespace std;

FlightsWindow::FlightsWindow(BlackBoxUI* blackBoxUI) : m_blackBoxUI(blackBoxUI)
{
    setWindowTitle("Flights");

    auto mainLayout = new QVBoxLayout();
    setLayout(mainLayout);

    m_tableWidget = new QTableWidget();
    m_tableWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    connect(m_tableWidget, &QTableWidget::cellDoubleClicked, this, [this](int row, int column)
    {
        printf("Double clicked row %d\n", row);
        auto flight = m_flightIndex.at(row);
        printf(" -> flight id=%llu\n", flight->id);

        m_blackBoxUI->getMainWindow()->selectFlight(flight->id);
    });
    mainLayout->addWidget(m_tableWidget);

    auto buttons = new QHBoxLayout();
    mainLayout->addLayout(buttons);
    auto mergeButton = new QPushButton("Merge");
    connect(mergeButton, &QPushButton::clicked, this, &FlightsWindow::mergeSelected);
    buttons->addWidget(mergeButton);
    auto deleteButton = new QPushButton("Delete");
    connect(deleteButton, &QPushButton::clicked, this, [this]()
    {
        int currentRow = m_tableWidget->currentRow();
        auto flight = m_flightIndex.at(currentRow);
        m_blackBoxUI->deleteFlight(flight);
    });
    buttons->addWidget(deleteButton);
}

void FlightsWindow::updateFlights()
{
    auto flights = m_blackBoxUI->getFlights();
    m_tableWidget->clear();
    m_tableWidget->setRowCount(flights.size());
    m_tableWidget->setColumnCount(5);
    m_tableWidget->setHorizontalHeaderLabels({"Date", "Origin", "Destination", "Type", "Updates"});
    m_tableWidget->setAlternatingRowColors(true);
    m_tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tableWidget->setShowGrid(false);

    int row = 0;
    for (const auto& flight : flights)
    {
        string origin = flight->origin;
        Airport originAirport;
        if (m_blackBoxUI->getNavigraph()->findAirport(origin, originAirport))
        {
            origin = originAirport.name + " (" + origin + ")";
        }

        string dest = flight->destination;
        Airport destAirport;
        if (m_blackBoxUI->getNavigraph()->findAirport(dest, destAirport))
        {
            dest = destAirport.name + " (" + dest + ")";
        }

        string aircraftType = AircraftTypes::getName(flight->icaoType);

        auto startTime = QDateTime::fromMSecsSinceEpoch(flight->startTime);

        m_tableWidget->setItem(row, 0, new QTableWidgetItem(startTime.toString(Qt::DateFormat::TextDate)));
        m_tableWidget->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(origin)));
        m_tableWidget->setItem(row, 2, new QTableWidgetItem(QString::fromStdString(dest)));
        m_tableWidget->setItem(row, 3, new QTableWidgetItem(QString::fromStdString(aircraftType)));
        m_tableWidget->setItem(row, 4, new QTableWidgetItem(QString::number(flight->stateCount)));

        m_flightIndex.insert_or_assign(row, flight);

        row++;
    }
    m_tableWidget->resizeColumnsToContents();


}

void FlightsWindow::mergeSelected()
{
    auto items = m_tableWidget->selectedItems();
    vector<shared_ptr<Flight>> flights;
    for (auto item : items)
    {
        if (item->column() == 0)
        {
            auto flight = m_flightIndex.at(item->row());
            flights.push_back(flight);
        }
    }

    MergeFlights mergeFlights(&(m_blackBoxUI->getDataStore()));
    mergeFlights.merge(flights);
}
