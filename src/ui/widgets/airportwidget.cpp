//
// Created by Ian Parker on 17/01/2026.
//

#include "airportwidget.h"

#include "airporttablemodel.h"

#include <QCompleter>
#include <QTreeView>
#include <QHeaderView>
#include <QStandardItemModel>

#include "ui/blackbox.h"


AirportWidget::AirportWidget(BlackBoxUI* blackBoxUI) : m_blackBoxUI(blackBoxUI)
{
    QCompleter* completer = new QCompleter(this);
    AirportTableModel* airportModel = new AirportTableModel(m_blackBoxUI);

    completer->setCompletionMode(QCompleter::PopupCompletion);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setModel(airportModel);
    completer->setFilterMode(Qt::MatchContains);

    QTreeView* treeView = new QTreeView(this);
    completer->setPopup(treeView);
    treeView->setRootIsDecorated(false);
    treeView->setAlternatingRowColors(true);
    treeView->setColumnHidden(1, true);
    treeView->header()->hide();
    treeView->header()->setStretchLastSection(false);
    treeView->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    treeView->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);

    setEditable(true);
    setInsertPolicy(QComboBox::NoInsert);
    setCompleter(completer);
    QComboBox::setModel(airportModel);

    connect(this->lineEdit(), &QLineEdit::editingFinished, [this, treeView]()
    {
        int count = treeView->model()->rowCount();
        printf("AirportWidget: lineEdit finished: Rows: %d, current index: %d\n", count,
            currentIndex());
        if (count == 1)
        {
            auto selected = treeView->model()->data(treeView->model()->index(0, 1)).toString();
            printf("AirportWidget: lineEdit finished: Only item: %s\n", selected.toStdString().c_str());
            if (selected != m_airportCode)
            {
                updateAirport(selected.toStdString());
            }
        }
        else
        {
            auto selected = treeView->model()->data(treeView->model()->index(currentIndex(), 1)).toString();
            printf("AirportWidget: lineEdit finished: Selected: %s\n", selected.toStdString().c_str());
        }

    });

    connect(this, &QComboBox::activated, [this](int index)
        {
        printf("AirportWidget: Activated: %d\n", index);
        if (index == 0)
        {
            printf("AirportWidget: Clearing\n");
            m_airportCode = "";
        updateAirport("");
            return;
        }
    auto m = model();
        QModelIndex codeIdx = m->index(index, 1);
            printf("AirportWidget: Setting to: %s\n", m->data(codeIdx).toString().toStdString().c_str());
        updateAirport(m->data(codeIdx).toString().toStdString());
        });
}


void AirportWidget::setAirport(const std::string &code)
{
    if (code == m_airportCode)
    {
        return;
    }

    m_airportCode = code;
    updateAirportDisplay();
}

void AirportWidget::updateAirportDisplay()
{
    QString qcode = QString::fromStdString(m_airportCode);
    auto m = model();
    for (int i = 0; i < m->rowCount(); i++)
    {
        QModelIndex codeIdx = m->index(i, 1);

        if (m->data(codeIdx).toString() == qcode)
        {
            QModelIndex stringIdx = m->index(i, 0);
            auto displayName = m->data(stringIdx).toString();
            printf("AirportWidget: Setting to: %s\n", displayName.toStdString().c_str());
            setCurrentText(displayName);
            break;
        }
    }
}

void AirportWidget::focusOutEvent(QFocusEvent* event)
{
    QComboBox::focusOutEvent(event);
    printf("AirportWidget: focusOutEvent\n");
    updateAirportDisplay();
}

void AirportWidget::updateAirport(const std::string &code)
{
    m_airportCode = code;
    updateAirportDisplay();

    emit airportChanged(code);
}
