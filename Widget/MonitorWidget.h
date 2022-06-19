#ifndef MONITORWIDGET_H
#define MONITORWIDGET_H

#include <QObject>
#include <QWidget>
#include <QTableView>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QMap>
#include <iterator>
#include <QSplitter>
#include <QPushButton>
#include <QModelIndex>
#include <QMessageBox>
#include <QPoint>
#include <QDebug>
#include <string.h>
#include <QHBoxLayout>
#include "Logger.h"
#include "PackMessage.hpp"
#include "FinTechUI/DragTabWidget/TabPageWidget.hpp"
#include "FinTechUI/Model/XHeaderView.hpp"
#include "FinTechUI/Model/XTableModel.hpp"
#include "FinTechUI/Model/XSortFilterProxyModel.hpp"
#include "FinTechUI/Model/YButtonDelegate.hpp"
#include "FinTechUI/Model/XButtonDelegate.hpp"
#include "FinTechUI/Model/XProgressDelegate.hpp"
#include "FinTechUI/Model/XComboBoxDelegate.hpp"
#include "FinTechUI/FilterWidget.hpp"
#include "HPPackClient.h"

static QColor GetColoStatusColor(const Message::TColoStatus& ColoStatus)
{
    QColor color;
    if(ColoStatus.LoadAverage.Min1 > ColoStatus.LoadAverage.CPUS * 0.8 ||
        ColoStatus.LoadAverage.Min5 > ColoStatus.LoadAverage.CPUS * 0.8 ||
        ColoStatus.LoadAverage.Min15 > ColoStatus.LoadAverage.CPUS * 0.8 || 
        ColoStatus.CPUUsage.IOWaitRate > 0.1 ||
        ColoStatus.CPUUsage.IrqRate > 0.1 ||
        ColoStatus.CPUUsage.SoftIrqRate > 0.1 ||
        ColoStatus.CPUUsage.UsedRate > 0.8 ||
        ColoStatus.MemoryInfo.UsedRate > 0.8 ||
        ColoStatus.DiskInfo.Mount1UsedRate > 0.8 ||
        ColoStatus.DiskInfo.Mount2UsedRate > 0.8)
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

static QColor GetAppStatusColor(const Message::TAppStatus& AppStatus)
{
    QColor color;
    if(QString(AppStatus.Status).contains("NoStart"))
    {
        // 深橙色 #FF8C00
        color = QColor("#FF8C00");
    }
    else if(QString(AppStatus.Status).contains("Stoped"))
    {
        // 橙红色 #FF4500
        color = QColor("#FF4500");
    }
    else
    {   // 深蓝绿色 #00CED1
        color = QColor("#00CED1");
    }
    return color;
}

class MonitorWidget : public FinTechUI::TabPageWidget
{
    Q_OBJECT
public:
    explicit MonitorWidget(QWidget* parent = NULL);
protected:
    void InitFilterWidget();
    void InitColoStatusTableView();
    void InitAppStatusTableView();
    void UpdateColoStatusTable(const Message::PackMessage& msg);
    void AppendRow(const Message::TColoStatus& ColoStatus);
    void UpdateRow(const Message::TColoStatus& ColoStatus);
    void UpdateAppStatusTable(const Message::PackMessage& msg);
    void AppendRow(const Message::TAppStatus& AppStatus);
    void UpdateRow(const Message::TAppStatus& AppStatus);
    void KillApp(const Message::TAppStatus& AppStatus);
    void StartApp(const Message::TAppStatus& AppStatus);
protected slots:
    void OnReceivedAppReport(const QList<Message::PackMessage>&);
    void OnKillAppButtonClicked(const QModelIndex& index);
    void OnStartAppButtonClicked(const QModelIndex& index);
    void OnSelectColoRow(const QModelIndex&);
    void OnFilterTable(const QVector<QStringList>& filter);
private:
    FinTechUI::FilterWidget* m_FilterWidget;
    QSplitter* m_Splitter;
    QSplitter* m_RightSplitter;
    QHBoxLayout* m_HBoxLayout;
    int m_SelectedColoRow;
    QMap<int, QStringList> m_Filter;
    QTableView* m_ColoStatusTableView;
    FinTechUI::XTableModel* m_ColoStatusTableModel;
    FinTechUI::XSortFilterProxyModel* m_ColoStatusProxyModel;
    QMap<QString, FinTechUI::XTableModelRow*> m_ColoStatusMap;
    QTableView* m_AppStatusTableView;
    FinTechUI::XTableModel* m_AppStatusTableModel;
    FinTechUI::XSortFilterProxyModel* m_AppStatusProxyModel;
    QMap<QString, FinTechUI::XTableModelRow*> m_AppStatusMap;
    QMap<QString, Message::PackMessage> m_AppStatusMessageMap;
    QMap<QString, QStringList> m_ColoAppSetMap;
    QMap<QString, QStringList> m_AppAccountSetMap;
};

#endif // MONITORWIDGET_H
