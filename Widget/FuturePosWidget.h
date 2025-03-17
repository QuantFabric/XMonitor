#ifndef FUTUREPOSWIDGET_H
#define FUTUREPOSWIDGET_H

#include <QObject>
#include <QWidget>
#include <QTableView>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QString>
#include <QStringList>
#include <QHeaderView>
#include <QSplitter>
#include <QMap>
#include <QVector>
#include <QList>
#include <QDebug>
#include "FMTLogger.hpp"
#include "PackMessage.hpp"
#include "FinTechUI/DragTabWidget/TabPageWidget.hpp"
#include "FinTechUI/Model/XTableModel.hpp"
#include "FinTechUI/Model/XSortFilterProxyModel.hpp"
#include "FinTechUI/FilterWidget.hpp"

struct FuturePosition
{
    int CloseYdShort;
    int CloseTdShort;
    int OpenLong;
    int CloseYdLong;
    int CloseTdLong;
    int OpenShort;

    FuturePosition()
    {
        this->CloseYdShort = 0;
        this->CloseTdShort = 0;
        this->OpenLong = 0;
        this->CloseYdLong = 0;
        this->CloseTdLong = 0;
        this->OpenShort = 0;
    }
};


class FuturePosWidget : public FinTechUI::TabPageWidget
{
    Q_OBJECT
public:
    explicit FuturePosWidget(const std::unordered_map<int, std::string>&, QWidget* parent = NULL);
protected slots:
    void OnReceivedOrderStatus(const QList<Message::PackMessage>&);
    void OnFilterTable(const QVector<QStringList>& filter);
protected:
    void UpdatePosition(const Message::PackMessage& msg);
    void AppendRow(const QString& Strategy, const Message::PackMessage& msg);
    void UpdateRow(const QString& Strategy, const Message::PackMessage& msg);
private:
    QHBoxLayout* m_HBoxLayout;
    QSplitter* m_Splitter;
    QWidget* m_LeftWidget;
    FinTechUI::FilterWidget* m_FilterWidget;
    QWidget* m_RightWidget;
    QTableView* m_PosTableView;
    FinTechUI::XTableModel* m_PosModel;
    FinTechUI::XSortFilterProxyModel* m_XSortFilterProxyModel;
    QMap<QString, FinTechUI::XTableModelRow*> m_StrategyAccountTickerModelRowMap;
    QMap<QString, FuturePosition> m_StrategyPositionMap;
    QMap<QString, QStringList> m_StrategyAccountSetMap;
    QMap<QString, QStringList> m_AccountTickerSetMap;
    QMap<int, QStringList> m_Filter;
    std::unordered_map<int, std::string> m_StrategyPropertyMap;
};

#endif // FUTUREPOSWIDGET_H
