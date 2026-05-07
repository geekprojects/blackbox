//
// Created by Ian Parker on 11/12/2025.
//

#ifndef BLACKBOX_SETTINGSWINDOW_H
#define BLACKBOX_SETTINGSWINDOW_H

#include <QWidget>

class BlackBoxUI;
class QLineEdit;

class SettingsWindow : public QWidget
{
    BlackBoxUI* m_blackBoxUI;

    QLineEdit* m_simPathEdit;
    QLineEdit* m_simbriefUsernameEdit;

 public:
    SettingsWindow(BlackBoxUI* blackBoxUI);
    ~SettingsWindow() override = default;

    void update();
};

#endif //BLACKBOX_SETTINGSWINDOW_H
