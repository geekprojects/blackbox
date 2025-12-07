//
// Created by Ian Parker on 01/12/2025.
//

#ifndef BLACKBOX_MAPMENU_H
#define BLACKBOX_MAPMENU_H
#include <QGeoView/QGVWidget.h>

#include <QToolButton>


class MapMenu : public QGVWidget
{
    Q_OBJECT

    QToolButton* m_mapButton;
    QWidget* m_menuWidget;

 public:
    MapMenu();
    ~MapMenu() override;


};


#endif //BLACKBOX_MAPMENU_H
