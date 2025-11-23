//
// Created by Ian Parker on 20/11/2025.
//

#ifndef BLACKBOX_MAINWINDOW_H
#define BLACKBOX_MAINWINDOW_H

#include <QMainWindow>

#include <QGeoView/QGVMap.h>
#include <QGeoView/QGVLayerGoogle.h>

#include "line.h"
#include "route.h"
#include "blackbox/datastore.h"

class MainWindow : public QMainWindow
{
    //Q_OBJECT
    QGVMap* mMap;
    QGVLayer* mItemsLayer;
    Line* m_line;
    Route* m_route;

    DataStore m_dataStore;
    uint64_t m_lastTimestamp = 0;

    State m_lastState;

public:
    MainWindow();
    ~MainWindow();

    void updateRoute();
};

#endif //BLACKBOX_MAINWINDOW_H
