#include "RiskJudgeWidget.h"
extern Utils::Logger *gLogger;

RiskJudgeWidget::RiskJudgeWidget(QWidget *parent) : FinTechUI::TabPageWidget(parent)
{
    InitFilterWidget();
    InitControlPannel();
    InitTickerCancelledTableView();
    InitAccountLockedTableView();
    InitRiskEventTableView();

    m_LeftSplitter = new QSplitter(Qt::Vertical);
    m_LeftSplitter->setHandleWidth(0);
    m_LeftSplitter->addWidget(m_FilterWidget);
    m_LeftSplitter->addWidget(m_ControlPannelWidget);
    m_LeftSplitter->setStretchFactor(0, 8);
    m_LeftSplitter->setStretchFactor(1, 2);

    m_MiddleSplitter = new QSplitter(Qt::Vertical);
    m_MiddleSplitter->setHandleWidth(0);
    m_MiddleSplitter->addWidget(m_TickerCancelledTableView);
    m_MiddleSplitter->addWidget(m_AccountLockedTableView);
    m_MiddleSplitter->setStretchFactor(0, 7);
    m_MiddleSplitter->setStretchFactor(1, 3);

    m_Splitter = new QSplitter(Qt::Horizontal);
    m_Splitter->addWidget(m_LeftSplitter);
    m_Splitter->addWidget(m_MiddleSplitter);
    m_Splitter->addWidget(m_RiskEventTableView);
    m_Splitter->setHandleWidth(2);
    m_Splitter->setStretchFactor(0, 1);
    m_Splitter->setStretchFactor(1, 4);
    m_Splitter->setStretchFactor(2, 7);
    m_Splitter->setCollapsible(0, true);
    m_Splitter->setCollapsible(2, true);

    QHBoxLayout* layout = new QHBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_Splitter);
    setLayout(layout);
    m_FlushTimer = startTimer(300, Qt::PreciseTimer);

    connect(m_FilterWidget, &FinTechUI::FilterWidget::FilterChanged, this, &RiskJudgeWidget::OnFilterTable, Qt::UniqueConnection);
}

void RiskJudgeWidget::OnReceivedRiskReport(const QList<Message::PackMessage>& messages)
{
    m_RiskReportQueue.append(messages);
}

void RiskJudgeWidget::OnFilterTable(const QVector<QStringList>& filter)
{
    QStringList RiskIDFilter = filter.at(0);
    QStringList AccountFilter = filter.at(1);
    QStringList TickerFilter = filter.at(2);
    m_Filter[0] = RiskIDFilter;
    m_Filter[1] = AccountFilter;
    m_Filter[2] = TickerFilter;
    m_TickerCancelledProxyModel->setRowFilter(m_Filter);
    m_TickerCancelledProxyModel->resetFilter();
    m_AccountLockedProxyModel->setRowFilter(m_Filter);
    m_AccountLockedProxyModel->resetFilter();
    m_RiskEventProxyModel->setRowFilter(m_Filter);
    m_RiskEventProxyModel->resetFilter();
}

void RiskJudgeWidget::InitFilterWidget()
{
    m_FilterWidget = new FinTechUI::FilterWidget;
    QStringList filter;
    filter << "RiskID" <<  "Account" << "Ticker";
    m_FilterWidget->SetHeaderLabels(filter);
    m_FilterWidget->SetColumnWidth("RiskID", 70);
    m_FilterWidget->SetColumnWidth("Account", 90);
    m_FilterWidget->SetColumnWidth("Ticker", 90);
}

void RiskJudgeWidget::InitControlPannel()
{
    m_ControlPannelWidget = new QWidget;
    QVBoxLayout* ControlPannelLayout = new QVBoxLayout;
    ControlPannelLayout->setContentsMargins(0, 0, 0, 0);
    {
        QGroupBox* limitGroup = new QGroupBox("RiskLimit");
        QVBoxLayout* vLayout = new QVBoxLayout;
        vLayout->setContentsMargins(0, 0, 0, 0);
        QFormLayout* fromLayout = new QFormLayout;
        fromLayout->setContentsMargins(0, 0, 0, 0);
        QComboBox* riskIDCombo = new QComboBox;
        fromLayout->addRow("RiskID:", riskIDCombo);
        connect(this, &RiskJudgeWidget::UpdateRiskIDColo, riskIDCombo,
                [ = ](const QMap<QString, QString>& map)
        {
            QStringList riskIDs;
            riskIDs.append(map.keys());
            riskIDs.sort();
            riskIDCombo->clear();
            riskIDCombo->addItems(riskIDs);
        });

        QLineEdit* flowLimitEdit = new QLineEdit;
        fromLayout->addRow("FlowLimit:", flowLimitEdit);
        QLineEdit* tickerCancelLimitEdit = new QLineEdit;
        fromLayout->addRow("TickerCancelLimit:", tickerCancelLimitEdit);
        QLineEdit* orderCancellLimitEdit = new QLineEdit;
        fromLayout->addRow("OrderCancelLimit:", orderCancellLimitEdit);

        vLayout->addLayout(fromLayout);
        QPushButton* submitButton = new QPushButton("Submit");
        vLayout->addWidget(submitButton);

        connect(riskIDCombo, &QComboBox::currentTextChanged,
                [ = ](const QString & riskID)
        {
            Message::TRiskReport& report = m_RiskLimitReportMap[riskID];
            flowLimitEdit->setText(QString::number(report.FlowLimit));
            tickerCancelLimitEdit->setText(QString::number(report.TickerCancelLimit));
            orderCancellLimitEdit->setText(QString::number(report.OrderCancelLimit));
        });

        connect(this, &RiskJudgeWidget::UpdateRiskLimit,
                [ = ](const Message::TRiskReport& report)
        {
            QString RiskID = riskIDCombo->currentText();
            flowLimitEdit->setText(QString::number(report.FlowLimit));
            tickerCancelLimitEdit->setText(QString::number(report.TickerCancelLimit));
            orderCancellLimitEdit->setText(QString::number(report.OrderCancelLimit));
        });

        connect(submitButton, &QAbstractButton::clicked,
                [ = ]()
        {
            QString RiskID = riskIDCombo->currentText();
            int FlowLimit = flowLimitEdit->text().toInt();
            int InstrumentCancelLimit = tickerCancelLimitEdit->text().toInt();
            int OrderCancellLimitEdit = orderCancellLimitEdit->text().toInt();
            QString command = QString::asprintf("RiskID:%s,FlowLimit:%d,TickerCancelLimit:%d,OrderCancelLimit:%d,Trader:%s",
                                                RiskID.toStdString().c_str(), FlowLimit, InstrumentCancelLimit,
                                                OrderCancellLimitEdit, "trader");
            Message::PackMessage message;
            message.MessageType = Message::EMessageType::ECommand;
            message.Command.CmdType = Message::ECommandType::EUPDATE_RISK_LIMIT;
            strncpy(message.Command.Colo, m_RiskIDColoMap[RiskID].toStdString().c_str(), sizeof(message.Command.Colo));
            strncpy(message.Command.Account, RiskID.toStdString().c_str(), sizeof(message.Command.Account));
            strncpy(message.Command.Command, command.toStdString().c_str(), sizeof(message.Command.Command));
            HPPackClient::SendData(reinterpret_cast<unsigned char *>(&message), sizeof(message));
            Utils::gLogger->Log->info("RiskJudgeWidget::InitControlPannel UpdateRiskLimit Colo:{} Account:{} Command:{}", 
                                        message.Command.Colo, message.Command.Account, message.Command.Command);
        });

        limitGroup->setLayout(vLayout);
        ControlPannelLayout->addWidget(limitGroup);
    }

    {
        QGroupBox* accountLockedGroup = new QGroupBox("LockedAccount");
        QVBoxLayout* vLayout = new QVBoxLayout;
        vLayout->setContentsMargins(0, 0, 0, 0);
        QFormLayout* fromLayout = new QFormLayout;
        fromLayout->setContentsMargins(0, 0, 0, 0);

        QComboBox* riskIDCombo = new QComboBox;
        fromLayout->addRow("RiskID:", riskIDCombo);
        connect(this, &RiskJudgeWidget::UpdateRiskIDAccounts, riskIDCombo,
                [ = ](const QMap<QString, QStringList>& map)
        {
            foreach(QString RiskID, map.keys())
            {
                foreach(QString Account, map[RiskID])
                {
                    QStringList& Accounts = m_RiskIDAccountsMap[RiskID];
                    if(!Accounts.contains(Account))
                    {
                        Accounts.append(Account);
                    }
                }
            }
            QStringList riskIDs;
            riskIDs.append(m_RiskIDAccountsMap.keys());
            riskIDs.sort();
            riskIDCombo->clear();
            riskIDCombo->addItems(riskIDs);
        });

        QComboBox* accountCombo = new QComboBox(accountLockedGroup);
        connect(riskIDCombo, &QComboBox::currentTextChanged, accountCombo,
                [ = ](const QString & text)
        {
            if(m_RiskIDAccountsMap.contains(text))
            {
                QStringList accounts = m_RiskIDAccountsMap[text];
                accounts.sort();
                accountCombo->clear();
                accountCombo->addItems(accounts);
            }
        });
        fromLayout->addRow("Account", accountCombo);

        QLineEdit* tickerEdit = new QLineEdit;
        fromLayout->addRow("Ticker:", tickerEdit);

        QComboBox* sideCombo = new QComboBox();
        QStringList sides;
        sides << "All" << "UnLock" << "Buy" << "Sell";
        sideCombo->addItems(sides);
        fromLayout->addRow("LockedSide:", sideCombo);
        vLayout->addLayout(fromLayout);

        QPushButton* submitButton = new QPushButton("Submit");
        vLayout->addWidget(submitButton);
        connect(submitButton, &QAbstractButton::clicked, accountCombo,
                [ = ]()
        {
            QString RiskID = riskIDCombo->currentText();
            QString Account = accountCombo->currentText();
            QString Ticker = tickerEdit->text();
            int LockSide = 0;
            if("All" == sideCombo->currentText())
            {
                LockSide = Message::ERiskLockedSide::ELOCK_ACCOUNT;
            }
            if("UnLock" == sideCombo->currentText())
            {
                LockSide = Message::ERiskLockedSide::EUNLOCK;
            }
            else if("Buy" == sideCombo->currentText())
            {
                LockSide = Message::ERiskLockedSide::ELOCK_BUY;
            }
            else if("Sell" == sideCombo->currentText())
            {
                LockSide = Message::ERiskLockedSide::ELOCK_SELL;
            }
            QString command = QString::asprintf("RiskID:%s,Account:%s,Ticker:%s,LockSide:%d,Trader:%s",
                                                RiskID.toStdString().c_str(), Account.toStdString().c_str(),
                                                Ticker.toStdString().c_str(), LockSide, "trader");
            Message::PackMessage message;
            message.MessageType = Message::EMessageType::ECommand;
            message.Command.CmdType = Message::ECommandType::EUPDATE_RISK_ACCOUNT_LOCKED;
            strncpy(message.Command.Colo, m_RiskIDColoMap[RiskID].toStdString().c_str(), sizeof(message.Command.Colo));
            strncpy(message.Command.Account, Account.toStdString().c_str(), sizeof(message.Command.Account));
            strncpy(message.Command.Command, command.toStdString().c_str(), sizeof(message.Command.Command));
            HPPackClient::SendData(reinterpret_cast<unsigned char *>(&message), sizeof(message));
            Utils::gLogger->Log->info("RiskJudgeWidget::InitControlPannel UpdateRiskAccountLocked Colo:{} Account:{} Command:{}", 
                                        message.Command.Colo, message.Command.Account, message.Command.Command);
        });

        accountLockedGroup->setLayout(vLayout);
        ControlPannelLayout->addWidget(accountLockedGroup);
    }
    ControlPannelLayout->setSpacing(10);
    ControlPannelLayout->addStretch(1);
    m_ControlPannelWidget->setLayout(ControlPannelLayout);
}

void RiskJudgeWidget::InitTickerCancelledTableView()
{
    QStringList headers;
    headers << "RiskID" << "Account" << "Ticker" << "CancelledCount" << "UpperLimit" << "Trader" << "UpdateTime";
    m_TickerCancelledTableView = new QTableView;
    m_TickerCancelledTableView->setSortingEnabled(true);
    m_TickerCancelledTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_TickerCancelledTableView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_TickerCancelledTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_TickerCancelledTableView->verticalHeader()->hide();
    m_TickerCancelledTableView->horizontalHeader()->setStretchLastSection(true);

    m_TickerCancelledTableModel = new FinTechUI::XTableModel;
    m_TickerCancelledTableModel->setObjectName("TickerCancelledTable");
    m_TickerCancelledTableModel->setHeaderLabels(headers);
    m_TickerCancelledProxyModel = new FinTechUI::XSortFilterProxyModel;
    m_TickerCancelledProxyModel->setSourceModel(m_TickerCancelledTableModel);
    m_TickerCancelledTableView->setModel(m_TickerCancelledProxyModel);
    m_TickerCancelledProxyModel->setDynamicSortFilter(true);
    m_TickerCancelledTableView->sortByColumn(6, Qt::DescendingOrder);

    int column = 0;
    m_TickerCancelledTableView->setColumnWidth(column++, 80);
    m_TickerCancelledTableView->setColumnWidth(column++, 100);
    m_TickerCancelledTableView->setColumnWidth(column++, 80);
    m_TickerCancelledTableView->setColumnWidth(column++, 105);
    m_TickerCancelledTableView->setColumnWidth(column++, 70);
    m_TickerCancelledTableView->setColumnWidth(column++, 70);
    m_TickerCancelledTableView->setColumnWidth(column++, 120);
}

void RiskJudgeWidget::InitAccountLockedTableView()
{
    QStringList headers;
    headers << "RiskID" << "Account" << "Ticker" << "LockedSide" << "Trader" << "UpdateTime";
    m_AccountLockedTableView = new QTableView;
    m_AccountLockedTableView->setSortingEnabled(true);
    m_AccountLockedTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_AccountLockedTableView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_AccountLockedTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_AccountLockedTableView->verticalHeader()->hide();
    m_AccountLockedTableView->horizontalHeader()->setStretchLastSection(true);

    m_AccountLockedTableModel = new FinTechUI::XTableModel;
    m_AccountLockedTableModel->setObjectName("AccountLockedTable");
    m_AccountLockedTableModel->setHeaderLabels(headers);
    m_AccountLockedProxyModel = new FinTechUI::XSortFilterProxyModel;
    m_AccountLockedProxyModel->setSourceModel(m_AccountLockedTableModel);
    m_AccountLockedTableView->setModel(m_AccountLockedProxyModel);
    m_TickerCancelledProxyModel->setDynamicSortFilter(true);
    m_AccountLockedTableView->sortByColumn(5, Qt::DescendingOrder);

    int column = 0;
    m_AccountLockedTableView->setColumnWidth(column++, 80);
    m_AccountLockedTableView->setColumnWidth(column++, 100);
    m_AccountLockedTableView->setColumnWidth(column++, 80);
    m_AccountLockedTableView->setColumnWidth(column++, 120);
    m_AccountLockedTableView->setColumnWidth(column++, 100);
    m_AccountLockedTableView->setColumnWidth(column++, 120);
}

void RiskJudgeWidget::InitRiskEventTableView()
{
    QStringList headers;
    headers << "RiskID" << "Account" << "Ticker" << "Trader" << "UpdateTime" << "Event";
    m_RiskEventTableView = new QTableView;
    m_RiskEventTableView->setSortingEnabled(true);
    m_RiskEventTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_RiskEventTableView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_RiskEventTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_RiskEventTableView->verticalHeader()->hide();
    m_RiskEventTableView->horizontalHeader()->setStretchLastSection(true);

    m_RiskEventTableModel = new FinTechUI::XTableModel;
    m_RiskEventTableModel->setObjectName("RiskEventTable");
    m_RiskEventTableModel->setHeaderLabels(headers);
    m_RiskEventProxyModel = new FinTechUI::XSortFilterProxyModel;
    m_RiskEventProxyModel->setSourceModel(m_RiskEventTableModel);
    m_RiskEventTableView->setModel(m_RiskEventProxyModel);
    m_RiskEventProxyModel->setDynamicSortFilter(true);
    m_RiskEventTableView->sortByColumn(4, Qt::DescendingOrder);

    int column = 0;
    m_RiskEventTableView->setColumnWidth(column++, 80);
    m_RiskEventTableView->setColumnWidth(column++, 100);
    m_RiskEventTableView->setColumnWidth(column++, 80);
    m_RiskEventTableView->setColumnWidth(column++, 80);
    m_RiskEventTableView->setColumnWidth(column++, 120);
    m_RiskEventTableView->setColumnWidth(column++, 1100);
}

void RiskJudgeWidget::HandleRiskReport(const Message::PackMessage& report)
{
    Utils::gLogger->Log->info("RiskJudgeWidget::HandleRiskReport ReportType:{} Colo:{} Broker:{} Product:{} Account:{} Ticker:{} RiskID:{}", 
                        report.RiskReport.ReportType, report.RiskReport.Colo, report.RiskReport.Broker, report.RiskReport.Product,
                        report.RiskReport.Account, report.RiskReport.Ticker, report.RiskReport.RiskID);
    switch(report.RiskReport.ReportType)
    {
    case Message::ERiskReportType::ERISK_TICKER_CANCELLED:
        UpdateTickerCancelTable(report);
        break;
    case Message::ERiskReportType::ERISK_LIMIT:
        UpdateRiskLimitTable(report);
        break;
    case Message::ERiskReportType::ERISK_ACCOUNT_LOCKED:
        UpdateLockedAccountTable(report);
        break;
    case Message::ERiskReportType::ERISK_EVENTLOG:
        UpdateRiskEventTable(report);
        break;
    }

    if(!m_RiskIDColoMap.contains(report.RiskReport.RiskID))
    {
        m_RiskIDColoMap[report.RiskReport.RiskID] = report.RiskReport.Colo;
        emit UpdateRiskIDColo(m_RiskIDColoMap);
    }

    if(Message::ERiskReportType::ERISK_LIMIT == report.RiskReport.ReportType)
    {
        return;
    }
    bool ok = false;
    QStringList& Accounts = m_RiskIDAccountsMap[report.RiskReport.RiskID];
    if(!Accounts.contains(report.RiskReport.Account))
    {
        Accounts.append(report.RiskReport.Account);
        ok = true;
    }
    QStringList& Tickers = m_AccountTickersMap[report.RiskReport.Account];
    if(!Tickers.contains(report.RiskReport.Ticker))
    {
        Tickers.append(report.RiskReport.Ticker);
        ok = true;
    }
    if(ok)
    {
        QVector<QMap<QString, QStringList>> data;
        data.append(m_RiskIDAccountsMap);
        data.append(m_AccountTickersMap);
        m_FilterWidget->SetDataRelationMap(data);
    }
}

void RiskJudgeWidget::UpdateTickerCancelTable(const Message::PackMessage& report)
{
    QString Account = report.RiskReport.Account;
    QString Ticker = report.RiskReport.Ticker;
    QString Key = Account + ":" + Ticker;
    auto it = m_TickerCancelledMap.find(Key);
    if(it != m_TickerCancelledMap.end())
    {
        FinTechUI::XTableModelRow* ModelRow = m_TickerCancelledMap[Key];
        char buffer[32] = {0};
        (*ModelRow)[3]->setText(report.RiskReport.CancelledCount);
        (*ModelRow)[4]->setText(report.RiskReport.UpperLimit);
        (*ModelRow)[5]->setText(report.RiskReport.Trader);
        (*ModelRow)[6]->setText(report.RiskReport.UpdateTime + 11);

        m_TickerCancelledTableModel->updateRow(ModelRow);
        FinTechUI::XTableModel::setRowBackgroundColor(ModelRow, GetRiskReportColor(report.RiskReport));
    }
    else
    {
        FinTechUI::XTableModelRow* ModelRow = new FinTechUI::XTableModelRow;
        FinTechUI::XTableModelItem* RiskIDItem = new FinTechUI::XTableModelItem(report.RiskReport.RiskID);
        ModelRow->push_back(RiskIDItem);
        FinTechUI::XTableModelItem* AccountItem = new FinTechUI::XTableModelItem(report.RiskReport.Account);
        ModelRow->push_back(AccountItem);
        FinTechUI::XTableModelItem* TickerItem = new FinTechUI::XTableModelItem(report.RiskReport.Ticker);
        ModelRow->push_back(TickerItem);
        FinTechUI::XTableModelItem* CancelledCountItem = new FinTechUI::XTableModelItem(report.RiskReport.CancelledCount);
        ModelRow->push_back(CancelledCountItem);
        FinTechUI::XTableModelItem* UpperLimitItem = new FinTechUI::XTableModelItem(report.RiskReport.UpperLimit);
        ModelRow->push_back(UpperLimitItem);
        FinTechUI::XTableModelItem* TraderItem = new FinTechUI::XTableModelItem(report.RiskReport.Trader);
        ModelRow->push_back(TraderItem);
        FinTechUI::XTableModelItem* UpdateTimeItem = new FinTechUI::XTableModelItem(report.RiskReport.UpdateTime + 11);
        ModelRow->push_back(UpdateTimeItem);

        FinTechUI::XTableModel::setRowBackgroundColor(ModelRow, GetRiskReportColor(report.RiskReport));

        m_TickerCancelledTableModel->appendRow(ModelRow);
        m_TickerCancelledMap[Key] = ModelRow;
    }
}

void RiskJudgeWidget::UpdateLockedAccountTable(const Message::PackMessage& report)
{
    QString Account = report.RiskReport.Account;
    auto it = m_AccountLockedMap.find(Account);
    if(it != m_AccountLockedMap.end())
    {
        FinTechUI::XTableModelRow* ModelRow = m_AccountLockedMap[Account];
        if(report.RiskReport.LockedSide != Message::ERiskLockedSide::EUNLOCK)
        {
            (*ModelRow)[3]->setText(GetLockedSide(report.RiskReport.LockedSide));
            (*ModelRow)[4]->setText(report.RiskReport.Trader);
            (*ModelRow)[5]->setText(report.RiskReport.UpdateTime + 11);
            FinTechUI::XTableModel::setRowBackgroundColor(ModelRow, GetRiskReportColor(report.RiskReport));
            m_AccountLockedTableModel->updateRow(ModelRow);
        }
        else
        {
            m_AccountLockedMap.erase(it);
            m_AccountLockedTableModel->deleteRow(ModelRow, true);
        }
    }
    else if(report.RiskReport.LockedSide != Message::ERiskLockedSide::EUNLOCK)
    {
        FinTechUI::XTableModelRow* ModelRow = new FinTechUI::XTableModelRow;
        FinTechUI::XTableModelItem* RiskIDItem = new FinTechUI::XTableModelItem(report.RiskReport.RiskID);
        ModelRow->push_back(RiskIDItem);
        FinTechUI::XTableModelItem* AccountItem = new FinTechUI::XTableModelItem(report.RiskReport.Account);
        ModelRow->push_back(AccountItem);
        FinTechUI::XTableModelItem* TickerItem = new FinTechUI::XTableModelItem(report.RiskReport.Ticker);
        ModelRow->push_back(TickerItem);
        FinTechUI::XTableModelItem* SideItem = new FinTechUI::XTableModelItem(GetLockedSide(report.RiskReport.LockedSide));
        ModelRow->push_back(SideItem);
        FinTechUI::XTableModelItem* TraderItem = new FinTechUI::XTableModelItem(report.RiskReport.Trader);
        ModelRow->push_back(TraderItem);
        FinTechUI::XTableModelItem* UpdateTimeItem = new FinTechUI::XTableModelItem(report.RiskReport.UpdateTime + 11);
        ModelRow->push_back(UpdateTimeItem);

        FinTechUI::XTableModel::setRowBackgroundColor(ModelRow, GetRiskReportColor(report.RiskReport));

        m_AccountLockedTableModel->appendRow(ModelRow);
        m_AccountLockedMap[Account] = ModelRow;
    }
}

void RiskJudgeWidget::UpdateRiskLimitTable(const Message::PackMessage& report)
{
    Message::TRiskReport& riskLimit = m_RiskLimitReportMap[report.RiskReport.RiskID];
    riskLimit = report.RiskReport;
    emit UpdateRiskLimit(riskLimit);
    Utils::gLogger->Log->info("RiskJudgeWidget::UpdateRiskLimitTable RiskID:{} FlowLimit:{} TickerCancelLimit:{} OrderCancelLimit:{}", 
                                        riskLimit.RiskID, riskLimit.FlowLimit, riskLimit.TickerCancelLimit, riskLimit.OrderCancelLimit);
}

void RiskJudgeWidget::UpdateRiskEventTable(const Message::PackMessage& report)
{
    FinTechUI::XTableModelRow* ModelRow = new FinTechUI::XTableModelRow;
    FinTechUI::XTableModelItem* RiskIDItem = new FinTechUI::XTableModelItem(report.RiskReport.RiskID);
    ModelRow->push_back(RiskIDItem);
    FinTechUI::XTableModelItem* AccountItem = new FinTechUI::XTableModelItem(report.RiskReport.Account);
    ModelRow->push_back(AccountItem);
    FinTechUI::XTableModelItem* TickerItem = new FinTechUI::XTableModelItem(report.RiskReport.Ticker);
    ModelRow->push_back(TickerItem);
    FinTechUI::XTableModelItem* TraderItem = new FinTechUI::XTableModelItem(report.RiskReport.Trader);
    ModelRow->push_back(TraderItem);
    FinTechUI::XTableModelItem* UpdateTimeItem = new FinTechUI::XTableModelItem(report.RiskReport.UpdateTime + 11);
    ModelRow->push_back(UpdateTimeItem);
    FinTechUI::XTableModelItem* EventItem = new FinTechUI::XTableModelItem(report.RiskReport.Event, Qt::AlignLeft | Qt::AlignVCenter);
    ModelRow->push_back(EventItem);

    FinTechUI::XTableModel::setRowBackgroundColor(ModelRow, GetRiskReportColor(report.RiskReport));

    m_RiskEventTableModel->appendRow(ModelRow);
    QMessageBox::warning(this, "Risk Warning", report.RiskReport.Event);
    Utils::gLogger->Log->info("RiskJudgeWidget::UpdateRiskEventTable RiskID:{} Account:{} Ticker:{} Event:{}", 
                                report.RiskReport.RiskID, report.RiskReport.Account, report.RiskReport.Ticker, report.RiskReport.Event);
}

void RiskJudgeWidget::timerEvent(QTimerEvent *event)
{
    // flush every 200ms
    if(event->timerId() == m_FlushTimer)
    {
        while(!m_RiskReportQueue.empty())
        {
            Message::PackMessage data;
            memcpy(&data, &m_RiskReportQueue.head(), sizeof(data));
            HandleRiskReport(data);
            m_RiskReportQueue.dequeue();
        }
    }
}
