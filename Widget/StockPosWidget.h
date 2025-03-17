#ifndef STOCKPOSWIDGET_H
#define STOCKPOSWIDGET_H

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

struct StockPosition
{
    int OpenLong; // 普通买入
    int ShortTdBuy; // 买券还券
    int MarginTdBuy; // 融资买入
    int CloseLong; // 普通卖出
    int ShortTdSell; // 融券卖出
    int MarginTdSell; // 卖券还款

    StockPosition()
    {
        this->OpenLong = 0;
        this->ShortTdBuy = 0;
        this->MarginTdBuy = 0;
        this->CloseLong = 0;
        this->ShortTdSell = 0;
        this->MarginTdSell = 0;
    }
};


class StockPosWidget : public FinTechUI::TabPageWidget
{
    Q_OBJECT
public:
    explicit StockPosWidget(const std::unordered_map<int, std::string>&, QWidget* parent = NULL);
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
    QMap<QString, StockPosition> m_StrategyPositionMap;
    QMap<QString, QStringList> m_StrategyAccountSetMap;
    QMap<QString, QStringList> m_AccountTickerSetMap;
    QMap<int, QStringList> m_Filter;
    std::unordered_map<int, std::string> m_StrategyPropertyMap;
};

#endif // STOCKPOSWIDGET_H
