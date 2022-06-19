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
#include "Logger.h"
#include "PackMessage.hpp"
#include "FinTechUI/DragTabWidget/TabPageWidget.hpp"
#include "FinTechUI/Model/XTableModel.hpp"
#include "FinTechUI/Model/XSortFilterProxyModel.hpp"
#include "FinTechUI/FilterWidget.hpp"
#include "HPPackClient.h"

static QString GetLockedSide(int side)
{
    QString buffer;
    switch (side)
    {
    case Message::ERiskLockedSide::EUNLOCK:
        buffer = "UnLock";
        break;
    case Message::ERiskLockedSide::ELOCK_BUY:
        buffer = "Buy";
        break;
    case Message::ERiskLockedSide::ELOCK_SELL:
        buffer = "Sell";
        break;
    case Message::ERiskLockedSide::ELOCK_ACCOUNT:
        buffer = "All";
        break;
    default:
        buffer = "Unkown Side";
        break;
    }
    return buffer;
}

static QColor GetRiskReportColor(const Message::TRiskReport& RiskReport)
{
    QColor color;
    if(Message::ERiskReportType::ERISK_TICKER_CANCELLED == RiskReport.ReportType 
        && RiskReport.CancelledCount > RiskReport.UpperLimit * 0.8)
    {
        // 深橙色 #FF8C00
        color = QColor("#FF8C00");
    }
    else if(Message::ERiskReportType::ERISK_ACCOUNT_LOCKED == RiskReport.ReportType
            && Message::ERiskLockedSide::ELOCK_BUY == RiskReport.LockedSide)
    {
        // 深橙色 #FF8C00
        color = QColor("#FF8C00");
    }
    else if(Message::ERiskReportType::ERISK_ACCOUNT_LOCKED == RiskReport.ReportType
            && Message::ERiskLockedSide::ELOCK_SELL == RiskReport.LockedSide)
    {
        // 深橙色 #FF8C00
        color = QColor("#FF8C00");
    }
    else if(Message::ERiskReportType::ERISK_ACCOUNT_LOCKED == RiskReport.ReportType
            && Message::ERiskLockedSide::ELOCK_ACCOUNT == RiskReport.LockedSide)
    {
        // 橙红色 #FF4500
        color = QColor("#FF4500");
    }
    else if(Message::ERiskReportType::ERISK_EVENTLOG == RiskReport.ReportType
            && QString(RiskReport.Event).startsWith("SQL"))
    {
        // 深蓝绿色 #00CED1
        color = QColor("#00CED1");
    }
    else
    {   // 深橙色 #FF8C00
        color = QColor("#FF8C00");
    }
    return color;
}

class RiskJudgeWidget : public FinTechUI::TabPageWidget
{
    Q_OBJECT
public:
    explicit RiskJudgeWidget(QWidget *parent = NULL);
protected slots:
    void OnReceivedRiskReport(const QList<Message::PackMessage>&);
    void OnFilterTable(const QVector<QStringList>& filter);
signals:
    void UpdateRiskIDAccounts(const QMap<QString, QStringList>&);
    void UpdateRiskIDColo(const QMap<QString, QString>&);
    void UpdateRiskLimit(const Message::TRiskReport&);
protected:
    void InitFilterWidget();
    void InitControlPannel();
    void InitTickerCancelledTableView();
    void InitAccountLockedTableView();
    void InitRiskEventTableView();
    void HandleRiskReport(const Message::PackMessage& report);
    void UpdateTickerCancelTable(const Message::PackMessage& report);
    void UpdateLockedAccountTable(const Message::PackMessage& report);
    void UpdateRiskLimitTable(const Message::PackMessage& report);
    void UpdateRiskEventTable(const Message::PackMessage& report);
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

    QTableView* m_AccountLockedTableView;
    FinTechUI::XTableModel* m_AccountLockedTableModel;
    FinTechUI::XSortFilterProxyModel* m_AccountLockedProxyModel;
    QMap<QString, FinTechUI::XTableModelRow*> m_AccountLockedMap;

    QTableView* m_TickerCancelledTableView;
    FinTechUI::XTableModel* m_TickerCancelledTableModel;
    FinTechUI::XSortFilterProxyModel* m_TickerCancelledProxyModel;
    QMap<QString, FinTechUI::XTableModelRow*> m_TickerCancelledMap;

    QTableView* m_RiskEventTableView;
    FinTechUI::XTableModel* m_RiskEventTableModel;
    FinTechUI::XSortFilterProxyModel* m_RiskEventProxyModel;

    QQueue<Message::PackMessage> m_RiskReportQueue;
    QMap<QString, Message::TRiskReport> m_RiskLimitReportMap;
    QMap<QString, QStringList> m_RiskIDAccountsMap;
    QMap<QString, QStringList> m_AccountTickersMap;
    QMap<QString, QString> m_RiskIDColoMap;
};

#endif // RISKJUDGEWIDGET_H
