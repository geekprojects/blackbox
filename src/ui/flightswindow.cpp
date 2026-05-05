//
// Created by Ian Parker on 03/01/2026.
//

#include "flightswindow.h"

#include <qdatetime.h>
#include <QHeaderView>
#include <QLineEdit>
#include <QPushButton>
#include <QSortFilterProxyModel>

#include "aircrafttypes.h"
#include "mainwindow.h"
#include "utils/mergeflights.h"

using namespace std;

FlightsWindow::FlightsWindow(BlackBoxUI* blackBoxUI) : m_blackBoxUI(blackBoxUI)
{
    setWindowTitle("Flights");

    auto mainLayout = new QVBoxLayout();
    setLayout(mainLayout);

    auto filterBox = new QHBoxLayout();
    mainLayout->addLayout(filterBox);
    filterBox->addWidget(new QLabel("Origin:"));
    filterBox->addWidget(m_originFilter = new QLineEdit());
    filterBox->addWidget(new QLabel("Destination:"));
    filterBox->addWidget(m_destFilter = new QLineEdit());
    filterBox->addWidget(new QLabel("Type:"));
    filterBox->addWidget(m_typeFilter = new QComboBox());

    m_tableWidget = new QTableView();
    m_tableWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);

    m_tableModel = new QStandardItemModel(this);
    m_sortModel = new QSortFilterProxyModel(this);
    m_sortModel->setSourceModel(m_tableModel);
    m_sortModel->setSortRole(Qt::UserRole);
    m_tableWidget->setModel(m_sortModel);
    m_tableWidget->setSortingEnabled(true);

    connect(m_tableWidget, &QTableView::doubleClicked, this, [this](const QModelIndex & index)
    {
        auto srcIndex = m_sortModel->mapToSource(index);
        printf("Double clicked row %d\n", srcIndex.row());
        auto flight = m_flightIndex.at(srcIndex.row());
        printf(" -> flight id=%llu\n", flight->id);

        m_blackBoxUI->getMainWindow()->showFlight(flight);
    });
    mainLayout->addWidget(m_tableWidget);

    m_tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    auto buttons = new QHBoxLayout();
    mainLayout->addLayout(buttons);
    auto mergeButton = new QPushButton("Merge");
    connect(mergeButton, &QPushButton::clicked, this, &FlightsWindow::mergeSelected);
    buttons->addWidget(mergeButton);
    auto deleteButton = new QPushButton("Delete");
    connect(deleteButton, &QPushButton::clicked, this, [this]()
    {
        for (auto const& flight : getSelectedFlights())
        {
            printf("Delete flight: %s -> %s\n", flight->origin.c_str(), flight->destination.c_str());
            m_blackBoxUI->deleteFlight(flight);
        }
    });
    buttons->addWidget(deleteButton);
}

void FlightsWindow::updateFlights()
{
    auto flights = m_blackBoxUI->getFlights();
    m_tableWidget->setAlternatingRowColors(true);
    m_tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tableWidget->setShowGrid(false);

    m_tableModel->clear();
    m_tableModel->setHorizontalHeaderLabels({"Date", "Origin", "Destination", "Type", "Updates"});
    m_tableModel->setColumnCount(5);
    m_tableModel->setRowCount(flights.size());

    set<string> types;
    int row = 0;
    for (const auto& flight : flights)
    {
        string aircraftType = AircraftTypes::getName(flight->icaoType);
        types.insert(aircraftType);

        m_flightIndex.insert_or_assign(row, flight);

        auto startTime = QDateTime::fromMSecsSinceEpoch(flight->startTime);
        auto dateItem = new QStandardItem(startTime.toString(Qt::DateFormat::TextDate));
        dateItem->setData(flight->startTime, Qt::UserRole);
        m_tableModel->setItem(row, 0, dateItem);

        string origin = flight->origin;
        Airport originAirport;
        if (m_blackBoxUI->getNavigraph()->findAirport(origin, originAirport))
        {
            origin = originAirport.name + " (" + origin + ")";
        }
        auto originItem = new QStandardItem(QString::fromStdString(origin));
        originItem->setData(QString::fromStdString(origin), Qt::UserRole);
        m_tableModel->setItem(row, 1, originItem);

        string dest = flight->destination;
        Airport destAirport;
        if (m_blackBoxUI->getNavigraph()->findAirport(dest, destAirport))
        {
            dest = destAirport.name + " (" + dest + ")";
        }
        auto destItem = new QStandardItem(QString::fromStdString(dest));
        destItem->setData(QString::fromStdString(dest), Qt::UserRole);
        m_tableModel->setItem(row, 2, destItem);

        auto typeItem = new QStandardItem(QString::fromStdString(aircraftType));
        typeItem->setData(QString::fromStdString(aircraftType), Qt::UserRole);
        m_tableModel->setItem(row, 3, typeItem);

        auto countItem = new QStandardItem(QString::number(flight->stateCount));
        countItem->setData(flight->stateCount, Qt::UserRole);
        m_tableModel->setItem(row, 4, countItem);

        row++;
    }
    m_tableWidget->resizeColumnsToContents();
    m_tableWidget->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
    m_tableWidget->adjustSize();

    m_typeFilter->clear();
    for (auto const& type : types)
    {
        m_typeFilter->addItem(QString::fromStdString(type));
    }

    adjustSize();
}

void FlightsWindow::mergeSelected()
{
    auto flights = getSelectedFlights();

    MergeFlights mergeFlights(&(m_blackBoxUI->getDataStore()));
    mergeFlights.merge(flights);
}

vector<std::shared_ptr<Flight>> FlightsWindow::getSelectedFlights()
{
    vector<std::shared_ptr<Flight>> flights;
    for (auto const& index : m_tableWidget->selectionModel()->selectedRows())
    {
        auto srcIndex = m_sortModel->mapToSource(index);
        auto flight = m_flightIndex.at(srcIndex.row());
        flights.push_back(flight);
    }
    return flights;
}
