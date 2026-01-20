//
// Created by Ian Parker on 17/01/2026.
//

#include "airporttablemodel.h"

#include "ui/blackbox.h"

AirportTableModel::AirportTableModel(BlackBoxUI* blackBoxUI) : m_blackBoxUI(blackBoxUI)
{
    auto airports = m_blackBoxUI->getNavigraph()->getAirports();
    setRowCount(airports.size() + 1);
    setColumnCount(2);
    for (int i = 0; i < airports.size(); i++)
    {
        QModelIndex searchIdx = QStandardItemModel::index(i + 1, 0);
        QModelIndex codeIdx = QStandardItemModel::index(i + 1, 1);
        QString code = QString::fromStdString(airports.at(i).code);
        QString name = QString::fromStdString(airports.at(i).name);
        QString searchText = name + " (" + code + ")";
        QStandardItemModel::setData(searchIdx, searchText);
        QStandardItemModel::setData(codeIdx, code);
    }
}
