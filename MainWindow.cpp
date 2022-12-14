#include "MainWindow.h"

extern Utils::Logger *gLogger;

MainWindow::MainWindow(const std::string& configPath, QWidget *parent): QWidget(parent)
{
    m_LoadStatus = false;
    std::string errorString;
    if(Utils::LoadXMonitorConfig(configPath.c_str(), m_XMonitorConfig, errorString))
    {
        Utils::gLogger->Log->info("LoadXMonitorConfig successed, XServerIP:{} XServerPort:{} ", m_XMonitorConfig.XServerIP, m_XMonitorConfig.XServerPort);
    }
    else
    {
        Utils::gLogger->Log->error("LoadXMonitorConfig failed, XServerIP:{} XServerPort:{} {}", m_XMonitorConfig.XServerIP, m_XMonitorConfig.XServerPort, errorString);
    }
    m_UserName = m_XMonitorConfig.UserName.c_str();
    m_PassWord = m_XMonitorConfig.PassWord.c_str();
    m_TabWidget = new FinTechUI::DragTabWidget(this);
    m_Layout = new QVBoxLayout;
    m_Layout->setContentsMargins(0, 0, 0, 0);
    m_Layout->addWidget(m_TabWidget);
    setLayout(m_Layout);
    setWindowTitle(QString("XMonitor [UserName:%1] Server[%2:%3]").arg(m_XMonitorConfig.UserName.c_str()).arg(m_XMonitorConfig.XServerIP.c_str()).arg(m_XMonitorConfig.XServerPort));
    int width = QApplication::desktop()->screenGeometry().width();
    int height = QApplication::desktop()->screenGeometry().height();
    move(20, 20);
    resize(width - 30, height - 100);
    m_HPPackClient = new HPPackClient(m_XMonitorConfig.XServerIP.c_str(), m_XMonitorConfig.XServerPort, m_XMonitorConfig.UserName.c_str(), m_XMonitorConfig.PassWord.c_str());
    connect(m_HPPackClient,
        SIGNAL(ReceivedLoginResponse(const Message::PackMessage&)),
        this,
        SLOT(OnReceivedLoginResponse(const Message::PackMessage&)));
    m_HPPackClient->Start();
}

void MainWindow::LoadPlugins()
{
    if(m_Plugins.contains(PLUGIN_PERMISSION))
    {
        m_TabWidget->addTabPage(m_PermissionWidget, PLUGIN_PERMISSION);
    }

    if(m_Plugins.contains(PLUGIN_MARKET))
    {
        m_MarketWidget = new MarketWidget();
        m_TabWidget->addTabPage(m_MarketWidget, PLUGIN_MARKET);
        connect(m_HPPackClient,
            SIGNAL(ReceivedFutureData(const QList<Message::PackMessage>&)),
            m_MarketWidget,
            SLOT(OnReceivedFutureData(const QList<Message::PackMessage>&)));
        connect(m_HPPackClient,
            SIGNAL(ReceivedSpotData(const QList<Message::PackMessage>&)),
            m_MarketWidget,
            SLOT(OnReceivedSpotData(const QList<Message::PackMessage>&)));
        connect(m_HPPackClient,
            SIGNAL(ReceivedStockData(const QList<Message::PackMessage>&)),
            m_MarketWidget,
            SLOT(OnReceivedStockData(const QList<Message::PackMessage>&)));
    }

    if(m_Plugins.contains(PLUGIN_EVENTLOG))
    {
        m_EventLogWidget = new EventLogWidget();
        m_TabWidget->addTabPage(m_EventLogWidget, PLUGIN_EVENTLOG);
        connect(m_HPPackClient,
            SIGNAL(ReceivedEventLog(const QList<Message::PackMessage>&)),
            m_EventLogWidget,
            SLOT(OnReceivedEventLog(const QList<Message::PackMessage>&)));
    }

    if(m_Plugins.contains(PLUGIN_RISKJUDGE))
    {
        m_RiskJudgeWidget = new RiskJudgeWidget();
        m_TabWidget->addTabPage(m_RiskJudgeWidget, PLUGIN_RISKJUDGE);
        connect(m_HPPackClient,
            SIGNAL(ReceivedRiskReport(const QList<Message::PackMessage>&)),
            m_RiskJudgeWidget,
            SLOT(OnReceivedRiskReport(const QList<Message::PackMessage>&)));
        connect(m_HPPackClient,
            SIGNAL(UpdateRiskIDAccounts(const QMap<QString, QStringList>&)),
            m_RiskJudgeWidget,
            SIGNAL(UpdateRiskIDAccounts(const QMap<QString, QStringList>&)));
    }

    if(m_Plugins.contains(PLUGIN_MONITOR))
    {
        m_MonitorWidget = new MonitorWidget();
        m_TabWidget->addTabPage(m_MonitorWidget, PLUGIN_MONITOR);
        connect(m_HPPackClient,
            SIGNAL(ReceivedAppReport(const QList<Message::PackMessage>&)),
            m_MonitorWidget,
            SLOT(OnReceivedAppReport(const QList<Message::PackMessage>&)));
    }

    if(m_Plugins.contains(PLUGIN_ORDERMANAGER))
    {
        m_OrderManagerWidget = new OrderManagerWidget();
        m_TabWidget->addTabPage(m_OrderManagerWidget, PLUGIN_ORDERMANAGER);
        connect(m_HPPackClient,
            SIGNAL(ReceivedExecutionReport(const QList<Message::PackMessage>&)),
            m_OrderManagerWidget,
            SLOT(OnReceivedExecutionReport(const QList<Message::PackMessage>&)));
    }
}

void MainWindow::OnReceivedLoginResponse(const Message::PackMessage& msg)
{
    if(!m_LoadStatus && m_UserName == msg.LoginResponse.Account)
    {
        if(msg.LoginResponse.ErrorID == 0)
        {
            QString Plugin = msg.LoginResponse.Plugins;
            m_Plugins = Plugin.split("|");
            if(m_Plugins.contains(PLUGIN_PERMISSION))
            {
                m_PermissionWidget = new PermissionWidget();
            }
            LoadPlugins();
            m_LoadStatus = true;
            m_HPPackClient->SetLoginSuccessed(true);
        }
    }
    if(m_Plugins.contains(PLUGIN_PERMISSION))
    {
        m_PermissionWidget->UpdateUserPermissionTable(msg);
    }
    Utils::gLogger->Log->info("MainWindow::OnReceivedLoginResponse Account:{} Role:{} Plugins:{} Messages:{} Operation:{}", 
                                msg.LoginResponse.Account, msg.LoginResponse.Role, msg.LoginResponse.Plugins, msg.LoginResponse.Messages, msg.LoginResponse.Operation);
}

MainWindow::~MainWindow()
{
    m_HPPackClient->Stop();
    delete m_HPPackClient;
    m_HPPackClient = NULL;
}

