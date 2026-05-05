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

class FlightsWindow : public QWidget
{
    BlackBoxUI* m_blackBoxUI = nullptr;

    QTableView* m_tableWidget = nullptr;
    //FlightsModel* m_tableModel = nullptr;
    QStandardItemModel* m_tableModel = nullptr;
    QSortFilterProxyModel* m_sortModel = nullptr;
    
    std::map<int, std::shared_ptr<Flight>> m_flightIndex;
    QLineEdit* m_originFilter;
    QLineEdit* m_destFilter;
    QComboBox* m_typeFilter;

    std::vector<std::shared_ptr<Flight>> getSelectedFlights();

 public:
    explicit FlightsWindow(BlackBoxUI* blackBoxUI);
    ~FlightsWindow() override = default;

    void updateFlights();

    void mergeSelected();
};


#endif //BLACKBOX_FLIGHTSWINDOW_H
