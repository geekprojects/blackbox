//
// Created by Ian Parker on 17/01/2026.
//

#ifndef BLACKBOX_AIRPORTTABLEMODEL_H
#define BLACKBOX_AIRPORTTABLEMODEL_H

#include <QStandardItemModel>

class BlackBoxUI;

class AirportTableModel : public QStandardItemModel
{
    BlackBoxUI* m_blackBoxUI;

 public:
     explicit AirportTableModel(BlackBoxUI* blackBoxUI);
    ~AirportTableModel() override = default;

};


#endif //BLACKBOX_AIRPORTTABLEMODEL_H
