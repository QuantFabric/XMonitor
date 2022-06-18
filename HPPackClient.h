#ifndef HPPACKCLIENT_H
#define HPPACKCLIENT_H

#include <string>
#include <stdio.h>
#include <string.h>
#include <cstdlib>
#include <vector>
#include <queue>
#include <QThread>
#include <QObject>
#include <QList>
#include <QMetaType>
#include <QDebug>
#include <QDateTime>
#include <QSet>
#include <QVariantMap>
#include <QMap>
#include <QString>
#include <QStringList>
#include <QMutex>
#include "HPSocket4C.h"
#include "PackMessage.hpp"
#include "Logger.h"

class HPPackClient: public QObject
{
    Q_OBJECT
public:
    HPPackClient(const char *ip, unsigned int port, const char* UserName, const char* PassWord);
    void Start();
    void SetLoginSuccessed(bool ok = true);
    void Stop();
    static void SendData(const unsigned char *pBuffer, int iLength);
    virtual ~HPPackClient();
private:
    void ConnectServer();
    void Login(const char* UserName, const char* PassWord);
signals:
    void ReceivedFutureData(const QList<Message::PackMessage>&);
    void ReceivedSpotData(const QList<Message::PackMessage>&);
    void ReceivedStockData(const QList<Message::PackMessage>&);
    void ReceivedEventLog(const QList<Message::PackMessage>&);
    void ReceivedExecutionReport(const QList<Message::PackMessage>&);
    void ReceivedRiskReport(const QList<Message::PackMessage>&);
    void ReceivedLoginResponse(const Message::PackMessage&);
    void ReceivedAppReport(const QList<Message::PackMessage>&);
    void UpdateRiskIDAccounts(const QMap<QString, QStringList>&);
protected slots:
    void WorkFunc();
protected:
    /***************************************************
     * HPSocket Call Back
     * *************************************************/
    static En_HP_HandleResult __stdcall OnConnect(HP_Client pSender, HP_CONNID dwConnID);
    static En_HP_HandleResult __stdcall OnSend(HP_Client pSender, HP_CONNID dwConnID, const BYTE *pData, int iLength);
    static En_HP_HandleResult __stdcall OnReceive(HP_Client pSender, HP_CONNID dwConnID, const BYTE *pData, int iLength);
    static En_HP_HandleResult __stdcall OnClose(HP_Client pSender, HP_CONNID dwConnID, En_HP_SocketOperation enOperation, int iErrorCode);
private:
    std::string m_ServerIP;// XServer IP
    unsigned int m_ServerPort;// XServer Port
    QString m_UserName;
    QString m_PassWord;
    HP_TcpPackClient m_pClient;// TCP PackClient
    HP_TcpPackClientListener m_pListener;// Listener
    static std::queue<Message::PackMessage> m_PackMessageQueue;
    static HP_Client hpClient;// HP Socket Client
    static bool m_IsConnected;
    QThread m_WorkThread;// thread
    static QMutex m_Mutex;
    QMap<QString, QVariantMap> m_ColoProductAccountMap;
    QMap<QString, QStringList> m_RiskIDAccountsMap;
    QMap<QString, QString> m_RiskIDColoMap;
    static int m_UUID;
    bool m_Stop;
    bool m_Login;
};


#endif // HPPACKCLIENT_H
