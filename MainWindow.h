#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <QTableWidget>
#include <QThread>
#include <QHBoxLayout>
#include <QMap>
#include <QTabWidget>
#include <QDebug>
#include <QSettings>
#include <QStringList>
#include <QApplication>
#include <QDesktopWidget>
#include "PackMessage.hpp"
#include "YMLConfig.hpp"
#include "FMTLogger.hpp"
#include "FinTechUI/DragTabWidget/DragTabWidget.hpp"
#include "HPPackClient.h"
#include "Widget/PermissionWidget.h"
#include "Widget/MarketWidget.h"
#include "Widget/EventLogWidget.h"
#include "Widget/RiskJudgeWidget.h"
#include "Widget/MonitorWidget.h"
#include "Widget/OrderManagerWidget.h"
#include "Widget/FuturePosWidget.h"
#include "Widget/StockPosWidget.h"

class MainWindow : public QWidget
{
    Q_OBJECT
public:
    MainWindow(const std::string& configPath, QWidget *parent = NULL);
    ~MainWindow();
protected:
    void LoadPlugins();
protected slots:
    void OnReceivedLoginResponse(const Message::PackMessage& msg);
private:
    QVBoxLayout* m_Layout;
    FinTechUI::DragTabWidget* m_TabWidget;
    PermissionWidget* m_PermissionWidget;
    MarketWidget* m_MarketWidget;
    EventLogWidget* m_EventLogWidget;
    RiskJudgeWidget* m_RiskJudgeWidget;
    MonitorWidget* m_MonitorWidget;
    OrderManagerWidget* m_OrderManagerWidget;
    FuturePosWidget* m_FuturePosWidget;
    StockPosWidget* m_StockPosWidget;

    Utils::XMonitorConfig m_XMonitorConfig;
    QString m_UserName;
    QString m_PassWord;
    bool m_LoadStatus;

    HPPackClient* m_HPPackClient;
    QStringList m_Plugins;
};

#endif // MAINWINDOW_H
