//
// Created by Ian Parker on 03/01/2026.
//

#ifndef BLACKBOX_FLIGHTSWINDOW_H
#define BLACKBOX_FLIGHTSWINDOW_H

#include <qcombobox.h>
#include <QSortFilterProxyModel>
#include <QWidget>
#include <QTableView>
#include <QStandardItemModel>
#include <QLabel>

#include "blackbox.h"

#if 0
class FlightsModel : public QAbstractTableModel
{
    Q_OBJECT

    BlackBoxUI* m_blackBoxUI;
    std::vector<std::shared_ptr<Flight>> m_flights;

 public:
    explicit FlightsModel(BlackBoxUI* blackBox, QObject* parent = nullptr);
    ~FlightsModel() override = default;

    void setFlights(std::vector<std::shared_ptr<Flight>> flights);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
};
#endif

class FlightProxyModel;

class FlightsWindow : public QWidget
{
    BlackBoxUI* m_blackBoxUI = nullptr;

    QTableView* m_tableWidget = nullptr;
    //FlightsModel* m_tableModel = nullptr;
    QStandardItemModel* m_tableModel = nullptr;
    FlightProxyModel* m_sortModel = nullptr;
    
    std::map<int, std::shared_ptr<Flight>> m_flightIndex;
    QComboBox* m_airportFilter;
    QComboBox* m_typeFilter;

    QLabel* m_flightCount;
    QLabel* m_landingAverage;

    std::vector<std::shared_ptr<Flight>> getSelectedFlights();

 public:
    explicit FlightsWindow(BlackBoxUI* blackBoxUI);
    ~FlightsWindow() override = default;

    void updateFlights();

    void mergeSelected();
};
class FlightProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

    // Filters
    std::string m_airport;
    std::string m_aircraftType;

 protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

 public:
    explicit FlightProxyModel(QObject *parent = nullptr) : QSortFilterProxyModel(parent)
    {
    }

    [[nodiscard]] std::string getAirport() const
    {
        return m_airport;
    }

    void setAirport(const std::string& airport)
    {
        beginFilterChange();
        this->m_airport = airport;
        endFilterChange(Direction::Rows);
    }

    [[nodiscard]] std::string getAircraftType() const
    {
        return m_aircraftType;
    }

    void setAircraftType(const std::string& aircraft_type)
    {
        beginFilterChange();
        m_aircraftType = aircraft_type;
        endFilterChange(Direction::Rows);
    }
};


#endif //BLACKBOX_FLIGHTSWINDOW_H
