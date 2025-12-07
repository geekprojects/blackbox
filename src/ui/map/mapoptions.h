//
// Created by Ian Parker on 02/12/2025.
//

#ifndef BLACKBOX_MAPOPTIONS_H
#define BLACKBOX_MAPOPTIONS_H

#include <QToolButton>

struct MapOption
{
    QString name;
    QIcon icon;
};

class MapOptions : public QToolButton
{
    std::vector<MapOption> m_options;
    QWidget* m_menuWidget = nullptr;

    void selectOption(bool selected);

    void adjustPopup();
    void resizeEvent(QResizeEvent *event);
    void moveEvent(QMoveEvent *event);

 public:
     explicit MapOptions(std::vector<MapOption> options);
     ~MapOptions() override = default;

    void setOptions(std::vector<MapOption> options);
    void setOption(const MapOption& option);
};


#endif //BLACKBOX_MAPOPTIONS_H
