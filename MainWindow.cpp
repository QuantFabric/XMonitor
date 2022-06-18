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

    m_HPPackClient->Start();
}

void MainWindow::LoadPlugins()
{

}

void MainWindow::OnReceivedLoginResponse(const Message::PackMessage& msg)
{

}

MainWindow::~MainWindow()
{
    m_HPPackClient->Stop();
    delete m_HPPackClient;
    m_HPPackClient = NULL;
}

