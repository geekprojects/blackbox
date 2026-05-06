//
// Created by Ian Parker on 03/01/2026.
//

#include "flightswindow.h"

#include <qdatetime.h>
#include <QHeaderView>
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
    filterBox->addWidget(new QLabel("Airport:"));
    filterBox->addWidget(m_airportFilter = new QComboBox());
    filterBox->addWidget(new QLabel("Type:"));
    filterBox->addWidget(m_typeFilter = new QComboBox());

    connect(m_airportFilter, &QComboBox::activated, this, [this](int index)
    {
        m_sortModel->setAirport(m_airportFilter->itemText(index).toStdString());
    });

    connect(m_typeFilter, &QComboBox::activated, this, [this](int index)
    {
        m_sortModel->setAircraftType(m_typeFilter->itemText(index).toStdString());
    });

    m_tableWidget = new QTableView();
    m_tableWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);

    m_tableModel = new QStandardItemModel(this);
    m_sortModel = new FlightProxyModel(this);
    m_sortModel->setSourceModel(m_tableModel);
    m_sortModel->setSortRole(Qt::UserRole);
    m_tableWidget->setModel(m_sortModel);
    m_tableWidget->setSortingEnabled(true);

    connect(m_tableWidget, &QTableView::doubleClicked, this, [this](const QModelIndex & index)
    {
        auto srcIndex = m_sortModel->mapToSource(index);
        auto flight = m_flightIndex.at(srcIndex.row());

        m_blackBoxUI->getMainWindow()->showFlight(flight);
    });
    mainLayout->addWidget(m_tableWidget);

    auto summary = new QHBoxLayout();
    mainLayout->addLayout(summary);
    summary->addWidget(new QLabel("Flights:"));
    summary->addWidget(m_flightCount = new QLabel());
    summary->addWidget(new QLabel("Landing:"));
    summary->addWidget(m_landingAverage = new QLabel());

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
    m_tableModel->setHorizontalHeaderLabels({"Date", "Origin", "Destination", "Type", "Landing", "Updates"});
    m_tableModel->setColumnCount(6);
    m_tableModel->setRowCount(flights.size());

    set<string> types;
    set<string> airports;
    int row = 0;
    double landingTotal = 0.0;
    int landingCount = 0;
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
        if (!origin.empty() && m_blackBoxUI->getNavigraph()->findAirport(origin, originAirport))
        {
            origin = originAirport.name + " (" + origin + ")";
        }
        if (!origin.empty())
        {
            airports.insert(origin);
        }
        auto originItem = new QStandardItem(QString::fromStdString(origin));
        originItem->setData(QString::fromStdString(origin), Qt::UserRole);
        m_tableModel->setItem(row, 1, originItem);

        string dest = flight->destination;
        Airport destAirport;
        if (!dest.empty() && m_blackBoxUI->getNavigraph()->findAirport(dest, destAirport))
        {
            dest = destAirport.name + " (" + dest + ")";
        }
        if (!dest.empty())
        {
            airports.insert(dest);
        }
        auto destItem = new QStandardItem(QString::fromStdString(dest));
        destItem->setData(QString::fromStdString(dest), Qt::UserRole);
        m_tableModel->setItem(row, 2, destItem);

        auto typeItem = new QStandardItem(QString::fromStdString(aircraftType));
        typeItem->setData(QString::fromStdString(aircraftType), Qt::UserRole);
        m_tableModel->setItem(row, 3, typeItem);

        QString landingStr;
        if (flight->landingRate < 0)
        {
            landingTotal += flight->landingRate;
            landingCount++;
            landingStr = QString::number(static_cast<int>(round(flight->landingRate)));
        }
        auto landingItem = new QStandardItem(landingStr);
        landingItem->setData(flight->landingRate, Qt::UserRole);
        m_tableModel->setItem(row, 4, landingItem);

        auto countItem = new QStandardItem(QString::number(flight->stateCount));
        countItem->setData(flight->stateCount, Qt::UserRole);
        m_tableModel->setItem(row, 5, countItem);

        row++;
    }
    m_tableWidget->resizeColumnsToContents();
    m_tableWidget->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
    m_tableWidget->adjustSize();

    double landingAverage = 0.0;
    if (landingCount > 0)
    {
        landingAverage = landingTotal / landingCount;
    }
    m_landingAverage->setText(QString::number(static_cast<int>(round(landingAverage))) + " fpm");

    m_airportFilter->clear();
    airports.insert("");
    for (auto const& airport : airports)
    {
        m_airportFilter->addItem(QString::fromStdString(airport));
    }

    m_typeFilter->clear();
    types.insert("");
    for (auto const& type : types)
    {
        m_typeFilter->addItem(QString::fromStdString(type));
    }

    adjustSize();
}

void FlightsWindow::mergeSelected()
{
    auto flights = getSelectedFlights();

    MergeFlights mergeFlights(&m_blackBoxUI->getDataStore());
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

bool FlightProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    if (!m_airport.empty())
    {
        auto index = sourceModel()->index(sourceRow, 1);
        auto origin = sourceModel()->data(index, Qt::DisplayRole).toString();
        index = sourceModel()->index(sourceRow, 2);
        auto dest = sourceModel()->data(index, Qt::DisplayRole).toString();
        if (!origin.contains(m_airport.c_str()) && !dest.contains(m_airport.c_str()))
        {
            return false;
        }
    }

    if (!m_aircraftType.empty())
    {
        auto index = sourceModel()->index(sourceRow, 3);
        auto type = sourceModel()->data(index, Qt::DisplayRole).toString();
        if (!type.contains(m_aircraftType.c_str()))
        {
            return false;
        }
    }

    return true;
}

