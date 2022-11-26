#include "HPPackClient.h"

Utils::LockFreeQueue<Message::PackMessage> HPPackClient::m_PackMessageQueue(1 << 15);
bool HPPackClient::m_IsConnected = false;
int HPPackClient::m_UUID = 0;

HP_Client HPPackClient::hpClient;
extern Utils::Logger* gLogger;

HPPackClient::HPPackClient(const char *ip, unsigned int port, const char* UserName, const char* PassWord)
{
    m_Stop = false;
    m_Login = false;
    m_ServerIP = ip;
    m_ServerPort = port;
    m_UserName = UserName;
    m_PassWord = PassWord;
    moveToThread(&m_WorkThread);
    connect(&m_WorkThread, SIGNAL(started()), this, SLOT(WorkFunc()));
    
    m_pListener = ::Create_HP_TcpPackClientListener();
    m_pClient = ::Create_HP_TcpPackClient(m_pListener);
    ::HP_Set_FN_Client_OnConnect(m_pListener, OnConnect);
    ::HP_Set_FN_Client_OnSend(m_pListener, OnSend);
    ::HP_Set_FN_Client_OnReceive(m_pListener, OnReceive);
    ::HP_Set_FN_Client_OnClose(m_pListener, OnClose);

    HP_TcpPackClient_SetMaxPackSize(m_pClient, 0xFFFF);
    HP_TcpPackClient_SetPackHeaderFlag(m_pClient, 0x169);
    HP_TcpClient_SetKeepAliveTime(m_pClient, 30 * 1000);

    qRegisterMetaType<Message::PackMessage>("Message::PackMessage");
    qRegisterMetaType<Message::PackMessage>("Message::PackMessage&");
    qRegisterMetaType<QList<Message::PackMessage>>("QList<Message::PackMessage>");
    qRegisterMetaType<QList<Message::PackMessage>>("QList<Message::PackMessage>&");
    qRegisterMetaType<QMap<QString, QVariantMap>>("QMap<QString, QVariantMap>");
    qRegisterMetaType<QMap<QString, QVariantMap>>("QMap<QString, QVariantMap>&");
    qRegisterMetaType<QMap<QString, QString>>("QMap<QString, QStringList>");
    qRegisterMetaType<QMap<QString, QString>>("QMap<QString, QStringList>&");
    qRegisterMetaType<QMap<QString, QString>>("QMap<QString, QString>");
    qRegisterMetaType<QMap<QString, QString>>("QMap<QString, QString>&");
}

void HPPackClient::Start()
{
    ConnectServer();
    m_WorkThread.start();
}

void HPPackClient::ConnectServer()
{
    if(m_IsConnected)
        return;
    if (::HP_Client_Start(m_pClient, (LPCTSTR)m_ServerIP.c_str(), m_ServerPort, false))
    {
        Login(m_UserName.toStdString().c_str(), m_PassWord.toStdString().c_str());
        Utils::gLogger->Log->info("HPPackClient::ConnectServer Client connect to server[{}:{}] successfully", m_ServerIP, m_ServerPort);
    }
    else
    {
        Utils::gLogger->Log->error("HPPackClient::ConnectServer Client connect to server[{}:{}] failed, error code:{} error massage:{}", m_ServerIP, m_ServerPort, HP_Client_GetLastError(m_pClient), (char*)HP_Client_GetLastErrorDesc(m_pClient));
        return;
    }
    QThread::msleep(1000);
}

void HPPackClient::Login(const char* UserName, const char* PassWord)
{
    Message::PackMessage message;
    message.MessageType = Message::EMessageType::ELoginRequest;
    message.LoginRequest.ClientType = Message::EClientType::EXMONITOR;
    strncpy(message.LoginRequest.Account, UserName, sizeof(message.LoginRequest.Account));
    strncpy(message.LoginRequest.PassWord, PassWord, sizeof(message.LoginRequest.PassWord));
    sprintf(message.LoginRequest.UUID, "%d", m_UUID);
    SendData((const unsigned char*)&message, sizeof (message));
}

void HPPackClient::SetLoginSuccessed(bool ok)
{
    m_Login = ok;
}

void HPPackClient::WorkFunc()
{
    while (true)
    {
        if(m_Stop)
        {
            break;
        }
        static qint64 prevTimeStamp = QDateTime::currentMSecsSinceEpoch();
        qint64 currentTimeStamp = QDateTime::currentMSecsSinceEpoch();
        if(currentTimeStamp - prevTimeStamp > 5000)
        {
            ConnectServer();
            prevTimeStamp =  QDateTime::currentMSecsSinceEpoch();
        }
        QList<Message::PackMessage> FutureDataList;
        QList<Message::PackMessage> SpotDataList;
        QList<Message::PackMessage> StockDataList;
        QList<Message::PackMessage> EventLogList;
        QList<Message::PackMessage> ExecuteReportList;
        QList<Message::PackMessage> RiskReportList;
        QList<Message::PackMessage> AppReportList;
        Message::PackMessage msg;
        while(m_PackMessageQueue.Pop(msg))
        {
            if(msg.MessageType == Message::EMessageType::ELoginResponse)
            {
                emit ReceivedLoginResponse(msg);
                continue;
            }
            while(!m_Login)
            {
                QThread::msleep(10);
            }
            if(msg.MessageType == Message::EMessageType::EEventLog)
            {
                EventLogList.append(msg);
            }
            else if(msg.MessageType == Message::EMessageType::EAccountFund)
            {
                ExecuteReportList.append(msg);
                QString Colo = msg.AccountFund.Colo;
                QString Product = msg.AccountFund.Product;
                QString Account = msg.AccountFund.Account;
                QVariantMap& ProductMap = m_ColoProductAccountMap[Colo];
                QStringList Accounts = ProductMap[Product].toStringList();
                if(!Accounts.contains(Account))
                {
                    Accounts.append(Account);
                    ProductMap[Product] = Accounts;
                    m_ColoProductAccountMap[Colo] = ProductMap;
                }
            }
            else if(msg.MessageType == Message::EMessageType::EAccountPosition)
            {
                ExecuteReportList.append(msg);
            }
            else if(msg.MessageType == Message::EMessageType::EOrderStatus)
            {
                ExecuteReportList.append(msg);
                QString RiskID = msg.OrderStatus.RiskID;
                QString Account = msg.OrderStatus.Account;
                QStringList& Accounts = m_RiskIDAccountsMap[RiskID];
                if(!Accounts.contains(Account))
                {
                    Accounts.append(Account);
                    emit UpdateRiskIDAccounts(m_RiskIDAccountsMap);
                }
            }
            else if(msg.MessageType == Message::EMessageType::ERiskReport)
            {
                RiskReportList.append(msg);
            }
            else if(msg.MessageType == Message::EMessageType::EColoStatus)
            {
                AppReportList.append(msg);
            }
            else if(msg.MessageType == Message::EMessageType::EAppStatus)
            {
                AppReportList.append(msg);
            }
            else if(msg.MessageType == Message::EMessageType::EFutureMarketData)
            {
                FutureDataList.append(msg);
            }
            else if(msg.MessageType == Message::EMessageType::ESpotMarketData)
            {
                SpotDataList.append(msg);
            }
            else if(msg.MessageType == Message::EMessageType::EStockMarketData)
            {
                StockDataList.append(msg);
            }
        }
        if(EventLogList.size() > 0)
        {
            emit ReceivedEventLog(EventLogList);
        }
        if(ExecuteReportList.size() > 0)
        {
            emit ReceivedExecutionReport(ExecuteReportList);
        }
        if(RiskReportList.size() > 0)
        {
            emit ReceivedRiskReport(RiskReportList);
        }
        if(AppReportList.size() > 0)
        {
            emit ReceivedAppReport(AppReportList);
        }
        if(FutureDataList.size() > 0)
        {
            emit ReceivedFutureData(FutureDataList);
        }
        if(SpotDataList.size() > 0)
        {
            emit ReceivedSpotData(SpotDataList);
        }
        if(StockDataList.size() > 0)
        {
            emit ReceivedStockData(StockDataList);
        }
        QThread::msleep(1);
    }
}

void HPPackClient::Stop()
{
    ::HP_Client_Stop(m_pClient);
    m_WorkThread.quit();
    m_Stop = true;
}

void HPPackClient::SendData(const unsigned char *pBuffer, int iLength)
{
    if(!m_IsConnected)
        return;
    bool ret = ::HP_Client_Send(hpClient, pBuffer, iLength);
    if(!ret)
    {
        Utils::gLogger->Log->warn("HPPackClient::SendData failed {} {} {}", ::HP_Client_GetLastError(hpClient), ::HP_Client_GetLastErrorDesc(hpClient), SYS_GetLastError());
    }
}

HPPackClient::~HPPackClient()
{
    ::Destroy_HP_TcpPackClient(m_pClient);
    ::Destroy_HP_TcpPackClientListener(m_pListener);
    m_WorkThread.wait();
}

En_HP_HandleResult __stdcall HPPackClient::OnConnect(HP_Client pSender, HP_CONNID dwConnID)
{
    TCHAR szAddress[50];
    int iAddressLen = sizeof(szAddress) / sizeof(TCHAR);
    USHORT usPort;
    ::HP_Client_GetLocalAddress(pSender, szAddress, &iAddressLen, &usPort);
    Utils::gLogger->Log->info("HPPackClient::OnConnect new connection:{}:{} connID:{}", szAddress, usPort, dwConnID);
    hpClient = pSender;
    m_IsConnected = true;
    m_UUID = usPort;
    return HR_OK;
}

En_HP_HandleResult __stdcall HPPackClient::OnSend(HP_Client pSender, HP_CONNID dwConnID, const BYTE *pData, int iLength)
{
    return HR_OK;
}

En_HP_HandleResult __stdcall HPPackClient::OnReceive(HP_Client pSender, HP_CONNID dwConnID, const BYTE *pData, int iLength)
{
    Message::PackMessage message;
    memcpy(&message, pData, sizeof(message));
    while(!m_PackMessageQueue.Push(message));
    char buffer[256] = {0};
    sprintf(buffer, "0X%X", message.MessageType);
    Utils::gLogger->Log->debug("HPPackClient::OnReceive MessageType:{}", buffer);
    return HR_OK;
}

En_HP_HandleResult __stdcall HPPackClient::OnClose(HP_Client pSender, HP_CONNID dwConnID, En_HP_SocketOperation enOperation, int iErrorCode)
{
    TCHAR szAddress[50];
    int iAddressLen = sizeof(szAddress) / sizeof(TCHAR);
    USHORT usPort;
    ::HP_Client_GetLocalAddress(pSender, szAddress, &iAddressLen, &usPort);
    Utils::gLogger->Log->info("HPPackClient::OnClose connection:{}:{} closed, connID:{}", szAddress, usPort, dwConnID);
    m_IsConnected = false;
    return HR_OK;
}


