#ifndef RISKJUDGEWIDGET_H
#define RISKJUDGEWIDGET_H

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
#include <QWidget>
#include <QDebug>
#include <QGroupBox>
#include <QVariant>
#include <QMessageBox>
#include <QVariantMap>
#include <QTableView>
#include "FMTLogger.hpp"
#include "PackMessage.hpp"
#include "FinTechUI/DragTabWidget/TabPageWidget.hpp"
#include "FinTechUI/Model/XTableModel.hpp"
#include "FinTechUI/Model/XSortFilterProxyModel.hpp"
#include "FinTechUI/FilterWidget.hpp"
#include "HPPackClient.h"


static QColor GetRiskReportColor(const Message::TRiskReport& RiskReport)
{
    QColor color;
    if(Message::ERiskReportType::ERISK_LIMIT == RiskReport.ReportType)
    {
        if(QString(RiskReport.Ticker).isEmpty())
        {
            if(RiskReport.OrderCount > RiskReport.OrderLimit * 0.8)
            {
                // 橙红色 #FF4500
                color = QColor("#FF4500");
            }
            else if(RiskReport.OrderCount > RiskReport.OrderLimit * 0.6)
            {
                // 深橙色 #FF8C00
                color = QColor("#FF8C00");
            }
            else 
            {
                // 深蓝绿色 #FF8C00
                color = QColor("#00CED1");
            }
        }
        else
        {
            if(RiskReport.CancelCount > RiskReport.CancelLimit * 0.8)
            {
                // 橙红色 #FF4500
                color = QColor("#FF4500");
            }
            else if(RiskReport.CancelCount > RiskReport.CancelLimit * 0.6)
            {
                // 深橙色 #FF8C00
                color = QColor("#FF8C00");
            }
            else 
            {
                // 深蓝绿色 #FF8C00
                color = QColor("#00CED1");
            }
        }
    }
    else if(Message::ERiskReportType::ERISK_POSITION_LIMIT == RiskReport.ReportType)
    {
        if(RiskReport.LongVolume > RiskReport.LongLimit * 0.8 || RiskReport.ShortVolume > RiskReport.ShortLimit * 0.8)
        {
            // 橙红色 #FF4500
            color = QColor("#FF4500");
        }
        else if(RiskReport.LongVolume > RiskReport.LongLimit * 0.6 || RiskReport.ShortVolume > RiskReport.ShortLimit * 0.6)
        {
            // 深橙色 #FF8C00
            color = QColor("#FF8C00");
        }
        else 
        {
            // 深蓝绿色 #FF8C00
            color = QColor("#00CED1");
        }
    }
    else if(Message::ERiskReportType::ERISK_ACCOUNT_LOCKED == RiskReport.ReportType)
    {
        if(RiskReport.LockSide == Message::EAccountLockSide::ELOCK_ACCOUNT)
        {
            // 橙红色 #FF4500
            color = QColor("#FF4500");
        }
        else if(RiskReport.LockSide == Message::EAccountLockSide::EUNLOCK)
        {
            // 深蓝绿色 #FF8C00
            color = QColor("#00CED1");
        }
        else 
        {
            // 深橙色 #FF8C00
            color = QColor("#FF8C00");
        }
    }
    else if(Message::ERiskReportType::ERISK_EVENTLOG == RiskReport.ReportType
            && QString(RiskReport.Event).startsWith("SQL"))
    {
        // 深蓝绿色 #00CED1
        color = QColor("#00CED1");
    }
    else if(Message::ERiskReportType::ERISK_EVENTLOG == RiskReport.ReportType
            && QString(RiskReport.Event).startsWith("ErrorMsg"))
    {
        // 橙红色 #FF4500
        color = QColor("#FF4500");
    }
    else
    {   
        // 深蓝绿色 #FF8C00
        color = QColor("#00CED1");
    }
    return color;
}

static QString GetBusinessType(int type)
{
    QString buffer;
    switch(type)
    {
    case Message::EBusinessType::ESTOCK:
        buffer = "股票";
        break;
    case Message::EBusinessType::ECREDIT:
        buffer = "信用";
        break;
    case Message::EBusinessType::EFUTURE:
        buffer = "期货";
        break;
    case Message::EBusinessType::ESPOT:
        buffer = "现货";
        break;
    default:
        buffer = "未知类型";
        break;
    }
    return buffer;
}

static QString GetLockSide(int lockside) 
{
    QString buffer;
    switch(lockside)
    {
    case Message::EAccountLockSide::EUNLOCK:
        buffer = "正常交易";
        break;
    case Message::EAccountLockSide::ELOCK_ACCOUNT:
        buffer = "禁止账户买卖";
        break;
    case Message::EAccountLockSide::ELOCK_OPEN:
        buffer = "禁止开仓";
        break;
    case Message::EAccountLockSide::ELOCK_CLOSE:
        buffer = "禁止平仓";
        break;
    case Message::EAccountLockSide::ELOCK_BUY:
        buffer = "禁止买入";
        break;
    case Message::EAccountLockSide::ELOCK_BUY_OPEN:
        buffer = "禁止买入开仓";
        break;
    case Message::EAccountLockSide::ELOCK_BUY_CLOSE:
        buffer = "禁止买入平仓";
        break;
    case Message::EAccountLockSide::ELOCK_BUY_CLOSE_TODAY:
        buffer = "禁止买入平今";
        break;
    case Message::EAccountLockSide::ELOCK_BUY_CLOSE_YESTODAY:
        buffer = "禁止买入平昨";
        break;
    case Message::EAccountLockSide::ELOCK_SELL:
        buffer = "禁止卖出";
        break;
    case Message::EAccountLockSide::ELOCK_SELL_OPEN:
        buffer = "禁止卖出开仓";
        break;
    case Message::EAccountLockSide::ELOCK_SELL_CLOSE:
        buffer = "禁止卖出平仓";
        break;
    case Message::EAccountLockSide::ELOCK_SELL_CLOSE_TODAY:
        buffer = "禁止卖出平今";
        break;
    case Message::EAccountLockSide::ELOCK_SELL_CLOSE_YESTODAY:
        buffer = "禁止卖出平昨";
        break;
    default:
        buffer = "未知类型";
        break;
    }
    return buffer;
}

class RiskJudgeWidget : public FinTechUI::TabPageWidget
{
    Q_OBJECT
public:
    explicit RiskJudgeWidget(QWidget *parent = NULL);
protected slots:
    void OnReceivedRiskReport(const QList<Message::PackMessage>&);
    void OnFilterTable(const QVector<QStringList>& filter);
    void OnUpdateRiskLimit(const QModelIndex &index);
    void OnUpdatePositionLimit(const QModelIndex &index);
    void OnUpdateAccountLocked(const QModelIndex &index);
signals:
    void UpdateRiskIDAccounts(const QMap<QString, QStringList>&);
    void UpdateRiskIDColo(const QMap<QString, QString>&);
    void UpdateRiskLimit(const Message::TRiskReport&);
protected:
    void InitFilterWidget();
    void InitRiskLimitTableView();
    void InitPositionLimitTableView();
    void InitAccountLockedTableView();
    void InitRiskEventTableView();
    void HandleRiskReport(const Message::PackMessage& report);
    void UpdateRiskLimitTable(const Message::PackMessage& report);
    void UpdateRiskEventTable(const Message::PackMessage& report);
    void UpdatePositionLimitTable(const Message::PackMessage& report);
    void UpdateAccountLockedTable(const Message::PackMessage& report);
protected:
    void timerEvent(QTimerEvent *event);
protected:
    QMap<int, QStringList> m_Filter;

    QSplitter* m_Splitter;
    int m_FlushTimer;
    QSplitter* m_LeftSplitter;
    FinTechUI::FilterWidget* m_FilterWidget;
    QWidget* m_ControlPannelWidget;
    QSplitter* m_MiddleSplitter;
    QSplitter* m_RightSplitter;

    QTableView* m_RiskLimitTableView;
    FinTechUI::XTableModel* m_RiskLimitTableModel;
    FinTechUI::XSortFilterProxyModel* m_RiskLimitProxyModel;
    QMap<QString, FinTechUI::XTableModelRow*> m_RiskLimitReportMap;

    QTableView* m_PositionLimitTableView;
    FinTechUI::XTableModel* m_PositionLimitTableModel;
    FinTechUI::XSortFilterProxyModel* m_PositionLimitProxyModel;
    QMap<QString, FinTechUI::XTableModelRow*> m_PositionLimitReportMap;

    QTableView* m_AccountLockedTableView;
    FinTechUI::XTableModel* m_AccountLockedTableModel;
    FinTechUI::XSortFilterProxyModel* m_AccountLockedProxyModel;
    QMap<QString, FinTechUI::XTableModelRow*> m_AccountLockedReportMap;

    QTableView* m_RiskEventTableView;
    FinTechUI::XTableModel* m_RiskEventTableModel;
    FinTechUI::XSortFilterProxyModel* m_RiskEventProxyModel;

    QQueue<Message::PackMessage> m_RiskReportQueue;
    
    QMap<QString, QStringList> m_RiskIDAccountsMap;
    QMap<QString, QStringList> m_AccountTickersMap;
    QMap<QString, QString> m_RiskIDColoMap;
};

#endif // RISKJUDGEWIDGET_H
