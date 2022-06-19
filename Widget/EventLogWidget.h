#ifndef EVENTLOGWIDGET_H
#define EVENTLOGWIDGET_H

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
#include "Logger.h"
#include "PackMessage.hpp"
#include "FinTechUI/DragTabWidget/TabPageWidget.hpp"
#include "FinTechUI/Model/XTableModel.hpp"
#include "FinTechUI/Model/XSortFilterProxyModel.hpp"
#include "FinTechUI/FilterWidget.hpp"

static QColor GetEventLogColor(const Message::TEventLog& EventLog)
{
    QColor color;
    if(Message::EEventLogLevel::EWARNING == EventLog.Level)
    {
        // 橙色 #FFA500
        color = QColor("#FFA500");
    }
    else if(Message::EEventLogLevel::EERROR == EventLog.Level)
    {
        // 橙红色 #FF4500
        color = QColor("#FF4500");
    }
    else if(Message::EEventLogLevel::EINFO == EventLog.Level)
    {   // 深蓝绿色 #00CED1
        color = QColor("#00CED1");
    }
    return color;
}

class EventLogWidget : public FinTechUI::TabPageWidget
{
    Q_OBJECT
public:
    explicit EventLogWidget(QWidget* parent = NULL);
protected slots:
    void OnReceivedEventLog(const QList<Message::PackMessage>&);
    void OnFilterTable(const QVector<QStringList>& filter);
protected:
    void AppendEventLog(const Message::PackMessage& msg);
private:
    QHBoxLayout* m_HBoxLayout;
    QSplitter* m_Splitter;
    QWidget* m_LeftWidget;
    FinTechUI::FilterWidget* m_FilterWidget;
    QWidget* m_RightWidget;
    QTableView* m_EventLogTableView;
    FinTechUI::XTableModel* m_EventLogModel;
    FinTechUI::XSortFilterProxyModel* m_XSortFilterProxyModel;
    QMap<QString, QStringList> m_ColoAppSetMap;
    QMap<QString, QStringList> m_AppAccountSetMap;
    QMap<QString, QStringList> m_AccountTickerSetMap;
    QMap<int, QStringList> m_Filter;
};

#endif // EVENTLOGWIDGET_H
