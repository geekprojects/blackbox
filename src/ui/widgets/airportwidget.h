//
// Created by Ian Parker on 17/01/2026.
//

#ifndef BLACKBOX_AIRPORTWIDGET_H
#define BLACKBOX_AIRPORTWIDGET_H
#include <QComboBox>
#include <QLineEdit>


class BlackBoxUI;

class AirportWidget : public QComboBox
{
    Q_OBJECT

    BlackBoxUI* m_blackBoxUI;

    std::string m_airportCode;

protected:
    void focusOutEvent(QFocusEvent* event) override;

    void updateAirport(const std::string& code);

 public:
    explicit AirportWidget(BlackBoxUI* blackBoxUI);
    ~AirportWidget() override = default;

    void updateAirportDisplay();

    void setAirport(const std::string& code);

    Q_SIGNALS:
        void airportChanged(const std::string& code);
};


#endif //BLACKBOX_AIRPORTWIDGET_H
