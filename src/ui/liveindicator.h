//
// Created by Ian Parker on 26/11/2025.
//

#ifndef BLACKBOX_LIVEINDICATOR_H
#define BLACKBOX_LIVEINDICATOR_H

#include <QWidget>

class LiveIndicator : public QWidget
{
    Q_OBJECT
    bool m_live = false;

 public:
    explicit LiveIndicator(QWidget* parent = nullptr);
    ~LiveIndicator() override = default;

    void paintEvent(QPaintEvent *) override;

    void setLive(bool live)
    {
        m_live = live;
        repaint();
    }
};


#endif //BLACKBOX_LIVEINDICATOR_H
