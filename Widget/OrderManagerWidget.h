#ifndef ORDERMANAGERWIDGET_H
#define ORDERMANAGERWIDGET_H

#include <QObject>
#include <QWidget>
#include <QString>
#include <QWidget>
#include <QTableWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QFormLayout>
#include <QComboBox>
#include <QTimerEvent>
#include <QHeaderView>
#include <QSplitter>
#include <QPushButton>
#include <QDialog>
#include <QLineEdit>
#include <QMultiMap>
#include <QDialogButtonBox>
#include <QQueue>
#include <QThread>
#include <QColor>
#include <QDebug>
#include <QFile>
#include <QMessageBox>
#include <QCoreApplication>
#include <QIntValidator>

#include "Logger.h"
#include "PackMessage.hpp"
#include "FinTechUI/DragTabWidget/TabPageWidget.hpp"
#include "FinTechUI/Model/XTableModel.hpp"
#include "FinTechUI/Model/XSortFilterProxyModel.hpp"
#include "FinTechUI/FilterWidget.hpp"
#include "FinTechUI/Model/FrozenTableView.hpp"
#include "HPPackClient.h"

static QString GetStockOrderSide(int side)
{
    QString buffer;
    switch (side)
    {
    case Message::EOrderSide::EOPEN_LONG:
        buffer = "普通买入";
        break;
    case Message::EOrderSide::ECLOSE_LONG:
        buffer = "普通卖出";
        break;
    case Message::EOrderSide::ESIDE_COLLATERAL_BUY:
        buffer = "担保品买入";
        break;
    case Message::EOrderSide::ESIDE_COLLATERAL_SELL:
        buffer = "担保品卖出";
        break;
    case Message::EOrderSide::ESIDE_REVERSE_REPO:
        buffer = "国债逆回购申购";
        break;
    case Message::EOrderSide::ESIDE_SUBSCRIPTION:
        buffer = "新股、新债申购";
        break;
    case Message::EOrderSide::ESIDE_ALLOTMENT:
        buffer = "配股配债认购";
        break;
    case Message::EOrderSide::ESIDE_MARGIN_BUY:
        buffer = "融资买入";
        break;
    case Message::EOrderSide::ESIDE_REPAY_MARGIN_BY_SELL:
        buffer = "卖券还款";
        break;
    case Message::EOrderSide::ESIDE_SHORT_SELL:
        buffer = "融券卖出";
        break;
    case Message::EOrderSide::ESIDE_REPAY_STOCK_BY_BUY:
        buffer = "买券还券";
        break;
    case Message::EOrderSide::ESIDE_REPAY_STOCK_DIRECT:
        buffer = "现券还券";
        break;
    default:
        buffer = "Unkown";
        break;
    }
    return buffer;
}

static QString GetFutureOrderSide(int side)
{
    QString buffer;
    switch (side)
    {
    case Message::EOrderSide::EOPEN_LONG:
        buffer = "OpenLong";
        break;
    case Message::EOrderSide::ECLOSE_TD_LONG:
        buffer = "CloseTdLong";
        break;
    case Message::EOrderSide::ECLOSE_YD_LONG:
        buffer = "CloseYdLong";
        break;
    case Message::EOrderSide::EOPEN_SHORT:
        buffer = "OpenShort";
        break;
    case Message::EOrderSide::ECLOSE_TD_SHORT:
        buffer = "CloseTdShort";
        break;
    case Message::EOrderSide::ECLOSE_YD_SHORT:
        buffer = "CloseYdShort";
        break;
    case Message::EOrderSide::ECLOSE_LONG:
        buffer = "CloseLong";
        break;
    case Message::EOrderSide::ECLOSE_SHORT:
        buffer = "CloseShort";
        break;
    default:
        buffer = "Unkown";
        break;
    }
    return buffer;
}

static QString GetOrderType(int type)
{
    QString buffer;
    switch (type)
    {
    case Message::EOrderType::EFAK:
        buffer = "FAK";
        break;
    case Message::EOrderType::EFOK:
        buffer = "FOK";
        break;
    case Message::EOrderType::ELIMIT:
        buffer = "LIMIT";
        break;
    case Message::EOrderType::EMARKET:
        buffer = "MARKET";
        break;
    default:
        buffer = "Unkown";
        break;
    }
    return buffer;
}

static QString GetOrderStatus(int status)
{
    QString buffer;
    switch (status)
    {
    case Message::EOrderStatus::EORDER_SENDED:
        buffer = QString::fromUtf8("订单已发出");
        break;
    case Message::EOrderStatus::EBROKER_ACK:
        buffer = QString::fromUtf8("柜台确认");
        break;
    case Message::EOrderStatus::EEXCHANGE_ACK:
        buffer = QString::fromUtf8("交易所确认");
        break;
    case Message::EOrderStatus::EPARTTRADED:
        buffer = QString::fromUtf8("部分成交");
        break;
    case Message::EOrderStatus::EALLTRADED:
        buffer = QString::fromUtf8("全部成交");
        break;
    case Message::EOrderStatus::ECANCELLING:
        buffer = QString::fromUtf8("正在撤单");
        break;
    case Message::EOrderStatus::ECANCELLED:
        buffer = QString::fromUtf8("撤单");
        break;
    case Message::EOrderStatus::EPARTTRADED_CANCELLED:
        buffer = QString::fromUtf8("部成部撤");
        break;
    case Message::EOrderStatus::EBROKER_ERROR:
        buffer = QString::fromUtf8("柜台错误");
        break;
    case Message::EOrderStatus::EEXCHANGE_ERROR:
        buffer = QString::fromUtf8("交易所错误");
        break;
    case Message::EOrderStatus::EACTION_ERROR:
        buffer = QString::fromUtf8("撤单错误");
        break;
    case Message::EOrderStatus::ERISK_ORDER_REJECTED:
        buffer = QString::fromUtf8("风控拒单");
        break;
    case Message::EOrderStatus::ERISK_ACTION_REJECTED:
        buffer = QString::fromUtf8("风控拒绝撤单");
        break;
    case Message::EOrderStatus::ERISK_CHECK_INIT:
        buffer = QString::fromUtf8("初始化检查");
        break;
    case Message::EOrderStatus::ERISK_CHECK_SELFMATCH:
        buffer = QString::fromUtf8("风控自成交");
        break;
    case Message::EOrderStatus::ERISK_CHECK_CANCELLIMIT:
        buffer = QString::fromUtf8("风控撤单限制");
        break;
    default:
        buffer = QString::fromUtf8("Unkown");
        break;
    }
    return buffer;
}

static QColor GetOrderStatusColor(const Message::TOrderStatus& OrderStatus)
{
    QColor color;
    if(Message::EOrderStatus::ERISK_CHECK_INIT == OrderStatus.OrderStatus)
    {
        if(OrderStatus.ErrorID == 0)
        {
            // 绿色
            color = QColor("#008000");
        }
        else
        {
            // 红色
            color = QColor("#FF0000");
        }
    }
    // EOrderSended 浅蓝色 #F0FFFF
    else if(Message::EOrderStatus::EORDER_SENDED == OrderStatus.OrderStatus)
    {
        color = QColor("#ADD8E6");
    }
    // EBrokerACK 浅绿色 #AFEEEE
    else if(Message::EOrderStatus::EBROKER_ACK == OrderStatus.OrderStatus)
    {
        color = QColor("#AFEEEE");
    }
    else if(Message::EOrderStatus::EEXCHANGE_ACK == OrderStatus.OrderStatus)
    {
        // EExchangeACK 绿松石 ＃40E0D0
        color = QColor("#40E0D0");
    }
    else if(Message::EOrderStatus::EPARTTRADED == OrderStatus.OrderStatus)
    {
        // EPartTraded 深天蓝 #00BFFF
        color = QColor("#00BFFF");
    }
    else if(Message::EOrderStatus::EALLTRADED == OrderStatus.OrderStatus)
    {
        // EAllTraded 深蓝绿色 #00CED1
        color = QColor("#00CED1");
    }
    else if(Message::EOrderStatus::ECANCELLING == OrderStatus.OrderStatus)
    {
        // ECancelling 橙色 #FFA500
        color = QColor("#FFA500");
    }
    else if(Message::EOrderStatus::ECANCELLED == OrderStatus.OrderStatus)
    {
        // ECancelled 深橙色 #FF8C00
        color = QColor("#FF8C00");
    }
    else if(Message::EOrderStatus::EPARTTRADED_CANCELLED == OrderStatus.OrderStatus)
    {
        // EPartTradedCancelled 深蓝色 ＃00BFFF
        color = QColor("#00BFFF");
    }
    else if(Message::EOrderStatus::EBROKER_ERROR == OrderStatus.OrderStatus)
    {
        // EExchangeError 橙红色 #FF4500
        color = QColor("#FF4500");
    }
    else if(Message::EOrderStatus::EEXCHANGE_ERROR == OrderStatus.OrderStatus)
    {
        // EExchangeError 橙红色 #FF4500
        color = QColor("#FF4500");
    }
    else if(Message::EOrderStatus::EACTION_ERROR == OrderStatus.OrderStatus)
    {
        // EActionError 橙红色 #FF4500
        color = QColor("#FF4500");
    }
    else if(Message::EOrderStatus::ERISK_ORDER_REJECTED == OrderStatus.OrderStatus)
    {
        // ERiskRejected 橙红色 #FF4500
        color = QColor("#FF4500");
    }
    else if(Message::EOrderStatus::ERISK_ACTION_REJECTED == OrderStatus.OrderStatus)
    {
        // ERiskCancelRejected 橙红色 #FF4500
        color = QColor("#FF4500");
    }
    else if(Message::EOrderStatus::ERISK_CHECK_SELFMATCH == OrderStatus.OrderStatus)
    {
        // ERiskCancelRejected 橙红色 #FF4500
        color = QColor("#FF4500");
    }
    else if(Message::EOrderStatus::ERISK_CHECK_CANCELLIMIT == OrderStatus.OrderStatus)
    {
        // ERiskCancelRejected 橙红色 #FF4500
        color = QColor("#FF4500");
    }
    return color;
}


static QColor GetAccountFundColor(const Message::TAccountFund& AccountFund)
{
    QColor color;
    if(AccountFund.Available < AccountFund.Balance * 0.2)
    {
        // 深橙色 #FF8C00
        color = QColor("#FF8C00");
    }
    else
    {   // 深蓝绿色 #00CED1
        color = QColor("#00CED1");
    }
    return color;
}

static QColor GetStockAccountPositionColor(const Message::TAccountPosition& AccountPosition)
{
    QColor color;
    if(AccountPosition.StockPosition.LongTdBuy > 0 
        || AccountPosition.StockPosition.LongTdSell > 0
        || AccountPosition.StockPosition.MarginTdBuy > 0
        || AccountPosition.StockPosition.MarginTdSell > 0
        || AccountPosition.StockPosition.ShortTdSell > 0
        || AccountPosition.StockPosition.ShortTdBuy > 0
        || AccountPosition.StockPosition.ShortDirectRepaid > 0)
    {
        // 橙色 #FFA500
        color = QColor("#FFA500");
    }
    else
    {   // 深蓝绿色 #00CED1
        color = QColor("#00CED1");
    }
    return color;
}

static QColor GetFutureAccountPositionColor(const Message::TAccountPosition& AccountPosition)
{
    QColor color;
    
    if(AccountPosition.FuturePosition.LongOpeningVolume > 0 
        || AccountPosition.FuturePosition.LongClosingTdVolume > 0
        || AccountPosition.FuturePosition.LongClosingYdVolume > 0
        || AccountPosition.FuturePosition.ShortOpeningVolume > 0
        || AccountPosition.FuturePosition.ShortClosingTdVolume > 0
        || AccountPosition.FuturePosition.ShortClosingYdVolume > 0)
    {
        // 橙红色 #FF4500
        color = QColor("#FF4500");
    }
    else if(AccountPosition.FuturePosition.LongTdVolume > 0 
        || AccountPosition.FuturePosition.ShortTdVolume > 0)
    {
        // 橙色 #FFA500
        color = QColor("#FFA500");
    }
    else
    {   // 深蓝绿色 #00CED1
        color = QColor("#00CED1");
    }
    return color;
}

class OrderManagerWidget : public FinTechUI::TabPageWidget
{
    Q_OBJECT
public:
    explicit OrderManagerWidget(QWidget* parent = NULL);
protected slots:
    void OnReceivedExecutionReport(const QList<Message::PackMessage>&);
    void OnSendOrder();
    void OnCancelOrder();
    void OnEndOrder(const QModelIndex &index);
    void OnFilterTable(const QVector<QStringList>& filter);
    void OnExportOrderTable();
    void OnTransferFund();
    void OnRepayMargin();
protected:
    void InitFilterWidget();
    void InitControlPannel();
    void InitStockTickerPositionTable();
    void InitFutureTickerPositionTable();
    void InitHangingOrderTable();
    void InitHistoryOrderTable();
    void InitAccountFundTable();
    void UpdateExecutionReport(const Message::PackMessage& msg);

    void UpdateAccountFund(const Message::PackMessage& msg);
    void AppendRow(const Message::TAccountFund& AccountFund);
    void UpdateAccountFund(const Message::TAccountFund& AccountFund);

    void UpdateStockAccountPosition(const Message::PackMessage& msg);
    void AppendStockRow(const Message::TAccountPosition& AccountPosition);
    void UpdateStockAccountPosition(const Message::TAccountPosition& AccountPosition);

    void UpdateFutureAccountPosition(const Message::PackMessage& msg);
    void AppendFutureRow(const Message::TAccountPosition& AccountPosition);
    void UpdateFutureAccountPosition(const Message::TAccountPosition& AccountPosition);
    
    void UpdateOrderStatus(const Message::PackMessage& msg);
    void UpdateHangingOrderTable(const Message::TOrderStatus& OrderStatus);
    void UpdateHistoryOrderTable(const Message::TOrderStatus& OrderStatus);
    void AppendRow(const Message::TOrderStatus& OrderStatus, FinTechUI::XTableModel* tableModel);
    void UpdateRow(const Message::TOrderStatus& OrderStatus, FinTechUI::XTableModel* tableModel);
    void RemoveRow(const Message::TOrderStatus& OrderStatus, FinTechUI::XTableModel* tableModel);

    void CancelOrderDialog(const QString& Account, const QString& OrderLocalID);
    void ExportTable(FinTechUI::XTableModel* tableModel, const QString& path);
protected:
    void timerEvent(QTimerEvent *event);
private:
    QWidget* m_LeftWidget;
    FinTechUI::FilterWidget* m_FilterWidget;
    QWidget* m_ControlPannelWidget;

    FinTechUI::FrozenTableView* m_StockTickerPositionTableView;
    FinTechUI::XTableModel* m_StockTickerPositionTableModel;
    QMap<QString, FinTechUI::XTableModelRow*> m_StockTickerPositionMap;
    FinTechUI::XSortFilterProxyModel* m_StockTickerPositionProxyModel;

    FinTechUI::FrozenTableView* m_FutureTickerPositionTableView;
    FinTechUI::XTableModel* m_FutureTickerPositionTableModel;
    QMap<QString, FinTechUI::XTableModelRow*> m_FutureTickerPositionMap;
    FinTechUI::XSortFilterProxyModel* m_FutureTickerPositionProxyModel;

    FinTechUI::FrozenTableView* m_HangingOrderTableView;
    FinTechUI::XTableModel* m_HangingOrderTableModel;
    QMap<QString, FinTechUI::XTableModelRow*> m_HangingOrderMap;
    FinTechUI::XSortFilterProxyModel* m_HangingOrderProxyModel;

    FinTechUI::FrozenTableView* m_HistoryOrderTableView;
    FinTechUI::XTableModel* m_HistoryOrderTableModel;
    FinTechUI::XSortFilterProxyModel* m_HistoryOrderProxyModel;

    QTableView* m_AccountFundTableView;
    FinTechUI::XTableModel* m_AccountFundTableModel;
    QMap<QString, FinTechUI::XTableModelRow*> m_AccountFundMap;
    FinTechUI::XSortFilterProxyModel* m_AccountFundProxyModel;

    QSplitter* m_Splitter;
    QSplitter* m_MiddleSplitter;
    int m_FlushTimer;
    QQueue<Message::PackMessage> m_ExecutionReportQueue;
    QMap<QString, QStringList> m_ColoProductSetMap;
    QMap<QString, QStringList> m_ProductAccountSetMap;
    QMap<QString, QStringList> m_AccountTickerSetMap;
    QSet<QString> m_AccountSet;
    QMap<QString, QString> m_AccountColoMap;
    QMap<QString, QString> m_ProductColoMap;
    QMap<QString, QString> m_AccountProductMap;
};

#endif // ORDERMANAGERWIDGET_H
