#include "RiskJudgeWidget.h"

RiskJudgeWidget::RiskJudgeWidget(QWidget *parent) : FinTechUI::TabPageWidget(parent)
{
    InitFilterWidget();
    InitRiskLimitTableView();
    InitPositionLimitTableView();
    InitAccountLockedTableView();
    InitRiskEventTableView();

    m_LeftSplitter = new QSplitter(Qt::Vertical);
    m_LeftSplitter->setHandleWidth(0);
    m_LeftSplitter->addWidget(m_FilterWidget);
    // m_LeftSplitter->setStretchFactor(0, 8);
    // m_LeftSplitter->setStretchFactor(1, 2);

    m_MiddleSplitter = new QSplitter(Qt::Vertical);
    m_MiddleSplitter->setHandleWidth(0);
    m_MiddleSplitter->addWidget(m_RiskLimitTableView);
    m_MiddleSplitter->addWidget(m_PositionLimitTableView);
    m_MiddleSplitter->setStretchFactor(0, 1);
    m_MiddleSplitter->setStretchFactor(1, 2);

    m_RightSplitter = new QSplitter(Qt::Vertical);
    m_RightSplitter->setHandleWidth(0);
    m_RightSplitter->addWidget(m_AccountLockedTableView);
    m_RightSplitter->addWidget(m_RiskEventTableView);
    m_RightSplitter->setStretchFactor(0, 1);
    m_RightSplitter->setStretchFactor(1, 2);
    
    m_Splitter = new QSplitter(Qt::Horizontal);
    m_Splitter->addWidget(m_LeftSplitter);
    m_Splitter->addWidget(m_MiddleSplitter);
    m_Splitter->addWidget(m_RightSplitter);
    m_Splitter->setHandleWidth(2);
    m_Splitter->setStretchFactor(0, 1);
    m_Splitter->setStretchFactor(1, 8);
    m_Splitter->setStretchFactor(2, 6);
    m_Splitter->setCollapsible(0, true);
    m_Splitter->setCollapsible(2, true);

    QHBoxLayout* layout = new QHBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_Splitter);
    setLayout(layout);
    m_FlushTimer = startTimer(300, Qt::PreciseTimer);

    connect(m_FilterWidget, &FinTechUI::FilterWidget::FilterChanged, this, &RiskJudgeWidget::OnFilterTable, Qt::UniqueConnection);

    connect(m_RiskLimitTableView, SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT(OnUpdateRiskLimit(const QModelIndex&)));
    connect(m_PositionLimitTableView, SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT(OnUpdatePositionLimit(const QModelIndex&)));
    connect(m_AccountLockedTableView, SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT(OnUpdateAccountLocked(const QModelIndex&)));
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
    m_RiskLimitProxyModel->setRowFilter(m_Filter);
    m_RiskLimitProxyModel->resetFilter();
    m_PositionLimitProxyModel->setRowFilter(m_Filter);
    m_PositionLimitProxyModel->resetFilter();
    m_AccountLockedProxyModel->setRowFilter(m_Filter);
    m_AccountLockedProxyModel->resetFilter();
    m_RiskEventProxyModel->setRowFilter(m_Filter);
    m_RiskEventProxyModel->resetFilter();
}

void RiskJudgeWidget::OnUpdateRiskLimit(const QModelIndex &index)
{
    int row = index.row();
    QString RiskID = m_RiskLimitProxyModel->index(row, 0).data().toString();
    QString Account = m_RiskLimitProxyModel->index(row, 1).data().toString();
    QString Ticker = m_RiskLimitProxyModel->index(row, 2).data().toString();
    QString Type = m_RiskLimitProxyModel->index(row, 3).data().toString();
    QString FlowLimit = m_RiskLimitProxyModel->index(row, 4).data().toString();
    QString CancelCount = m_RiskLimitProxyModel->index(row, 5).data().toString();
    QString CancelLimit = m_RiskLimitProxyModel->index(row, 6).data().toString();
    QString OrderCount = m_RiskLimitProxyModel->index(row, 7).data().toString();
    QString OrderLimit = m_RiskLimitProxyModel->index(row, 8).data().toString();
    QString OrderCancelLimit = m_RiskLimitProxyModel->index(row, 9).data().toString();
    QString Trader = m_RiskLimitProxyModel->index(row, 10).data().toString();

    QDialog dialog;
    dialog.setWindowTitle("Send Risk Limit Command");
    dialog.setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint | Qt::WindowStaysOnTopHint);
    dialog.setFixedSize(400, 500);

    QFormLayout* formLayout = new QFormLayout;
    formLayout->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);

    QLineEdit* RiskIDEdit = new QLineEdit;
    RiskIDEdit->setText(RiskID);
    formLayout->addRow("RiskID: ", RiskIDEdit);
    
    QLineEdit* AccountEdit = new QLineEdit;
    AccountEdit->setText(Account);
    formLayout->addRow("Account: ", AccountEdit);

    QLineEdit* TickerEdit = new QLineEdit;
    TickerEdit->setText(Ticker);
    formLayout->addRow("Ticker: ", TickerEdit);
    
    QComboBox* TypeCombo = new QComboBox;
    TypeCombo->addItem("股票", Message::EBusinessType::ESTOCK);
    TypeCombo->addItem("信用", Message::EBusinessType::ECREDIT);
    TypeCombo->addItem("期货", Message::EBusinessType::EFUTURE);
    TypeCombo->addItem("现货", Message::EBusinessType::ESPOT);
    TypeCombo->setCurrentText(Type);
    formLayout->addRow("Type: ", TypeCombo);

    QLineEdit* FlowLimitEdit = new QLineEdit;
    FlowLimitEdit->setText(FlowLimit);
    formLayout->addRow("FlowLimit: ", FlowLimitEdit);

    QLineEdit* CancelCountEdit = new QLineEdit;
    CancelCountEdit->setText(CancelCount);
    formLayout->addRow("CancelCount: ", CancelCountEdit);

    QLineEdit* CancelLimitEdit = new QLineEdit;
    CancelLimitEdit->setText(CancelLimit);
    formLayout->addRow("CancelLimit: ", CancelLimitEdit);

    QLineEdit* OrderCountEdit = new QLineEdit;
    OrderCountEdit->setText(OrderCount); 
    formLayout->addRow("OrderCount: ", OrderCountEdit);

    QLineEdit* OrderLimitEdit = new QLineEdit;
    OrderLimitEdit->setText(OrderLimit);
    formLayout->addRow("OrderLimit: ", OrderLimitEdit);

    QLineEdit* OrderCancelLimitEdit = new QLineEdit;
    OrderCancelLimitEdit->setText(OrderCancelLimit);
    formLayout->addRow("OrderCancelLimit: ", OrderCancelLimitEdit);

    QLineEdit* TraderEdit = new QLineEdit;
    TraderEdit->setText(Trader);
    formLayout->addRow("Trader: ", TraderEdit);

    QDialogButtonBox* buttonBox = new QDialogButtonBox;
    buttonBox->setCenterButtons(true);
    buttonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    formLayout->addWidget(buttonBox);
    connect(buttonBox, SIGNAL(accepted()), &dialog, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), &dialog, SLOT(reject()));

    dialog.setLayout(formLayout);
    if(dialog.exec() == QDialog::Accepted)
    {
        int BusinessType = TypeCombo->currentData().toInt();
        QString command = QString::asprintf("RiskID:%s,Account:%s,Ticker:%s,BusinessType:%d,FlowLimit:%d,CancelCount:%d,CancelLimit:%d,OrderCount:%d,OrderLimit:%d,OrderCancelLimit:%d,Trader:%s",                                                
                                            RiskIDEdit->text().toStdString().c_str(), AccountEdit->text().toStdString().c_str(), TickerEdit->text().toStdString().c_str(), BusinessType, 
                                            FlowLimitEdit->text().toInt(), CancelCountEdit->text().toInt(), CancelLimitEdit->text().toInt(), OrderCountEdit->text().toInt(), 
                                            OrderLimitEdit->text().toInt(), OrderCancelLimitEdit->text().toInt(), Trader.toStdString().c_str());            
        if(QMessageBox::Yes == QMessageBox::question(this, "Send Risk Limit Command", command))
        {
            Message::PackMessage message;            
            message.MessageType = Message::EMessageType::ECommand;            
            message.Command.CmdType = Message::ECommandType::EUPDATE_RISK_LIMIT;            
            strncpy(message.Command.Colo, m_RiskIDColoMap[RiskID].toStdString().c_str(), sizeof(message.Command.Colo));            
            strncpy(message.Command.Account, Account.toStdString().c_str(), sizeof(message.Command.Account));            
            strncpy(message.Command.Command, command.toStdString().c_str(), sizeof(message.Command.Command));            
            HPPackClient::SendData(reinterpret_cast<unsigned char *>(&message), sizeof(message));            
            FMTLOG(fmtlog::INF, "RiskJudgeWidget::OnUpdateRiskLimit Colo:{} Account:{} Command:{}",                     
                    message.Command.Colo, message.Command.Account, message.Command.Command);
        }
    }
}

void RiskJudgeWidget::OnUpdatePositionLimit(const QModelIndex &index)
{
    int row = index.row();
    QString RiskID = m_PositionLimitProxyModel->index(row, 0).data().toString();
    QString Account = m_PositionLimitProxyModel->index(row, 1).data().toString();
    QString Ticker = m_PositionLimitProxyModel->index(row, 2).data().toString();
    QString EngineID = m_PositionLimitProxyModel->index(row, 3).data().toString();
    QString Type = m_PositionLimitProxyModel->index(row, 4).data().toString();
    QString LongVolume = m_PositionLimitProxyModel->index(row, 5).data().toString();
    QString ShortVolume = m_PositionLimitProxyModel->index(row, 6).data().toString();
    QString LongLimit = m_PositionLimitProxyModel->index(row, 7).data().toString();
    QString ShortLimit = m_PositionLimitProxyModel->index(row, 8).data().toString();
    QString ExLowerLimit = m_PositionLimitProxyModel->index(row, 9).data().toString();
    QString ExUpperLimit = m_PositionLimitProxyModel->index(row, 10).data().toString();
    QString Trader = m_PositionLimitProxyModel->index(row, 11).data().toString();

    QDialog dialog;
    dialog.setWindowTitle("Send Position Limit Command");
    dialog.setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint | Qt::WindowStaysOnTopHint);
    dialog.setFixedSize(400, 500);

    QFormLayout* formLayout = new QFormLayout;
    formLayout->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);

    QLineEdit* RiskIDEdit = new QLineEdit;
    RiskIDEdit->setText(RiskID);
    formLayout->addRow("RiskID: ", RiskIDEdit);
    
    QLineEdit* AccountEdit = new QLineEdit;
    AccountEdit->setText(Account);
    formLayout->addRow("Account: ", AccountEdit);

    QLineEdit* TickerEdit = new QLineEdit;
    TickerEdit->setText(Ticker);
    formLayout->addRow("Ticker: ", TickerEdit);

    QLineEdit* EngineIDEdit = new QLineEdit;
    EngineIDEdit->setText(EngineID);
    formLayout->addRow("EngineID: ", EngineIDEdit);

    QComboBox* TypeCombo = new QComboBox;
    TypeCombo->addItem("股票", Message::EBusinessType::ESTOCK);
    TypeCombo->addItem("信用", Message::EBusinessType::ECREDIT);
    TypeCombo->addItem("期货", Message::EBusinessType::EFUTURE);
    TypeCombo->addItem("现货", Message::EBusinessType::ESPOT);
    TypeCombo->setCurrentText(Type);
    formLayout->addRow("Type: ", TypeCombo);

    QLineEdit* LongVolumeEdit = new QLineEdit;
    LongVolumeEdit->setText(LongVolume);
    formLayout->addRow("LongVolume: ", LongVolumeEdit);

    QLineEdit* ShortVolumeEdit = new QLineEdit;
    ShortVolumeEdit->setText(ShortVolume);
    formLayout->addRow("ShortVolume: ", ShortVolumeEdit);

    QLineEdit* LongLimitEdit = new QLineEdit;
    LongLimitEdit->setText(LongLimit);
    formLayout->addRow("LongLimit: ", LongLimitEdit);

    QLineEdit* ShortLimitEdit = new QLineEdit;
    ShortLimitEdit->setText(ShortLimit);
    formLayout->addRow("ShortLimit: ", ShortLimitEdit);

    QLineEdit* ExLowerLimitEdit = new QLineEdit;
    ExLowerLimitEdit->setText(ExLowerLimit);
    formLayout->addRow("ExLowerLimit: ", ExLowerLimitEdit);

    QLineEdit* ExUpperLimitEdit = new QLineEdit;
    ExUpperLimitEdit->setText(ExUpperLimit);
    formLayout->addRow("ExUpperLimit: ", ExUpperLimitEdit);

    QLineEdit* TraderEdit = new QLineEdit;
    TraderEdit->setText(Trader);
    formLayout->addRow("Trader: ", TraderEdit);

    QDialogButtonBox* buttonBox = new QDialogButtonBox;
    buttonBox->setCenterButtons(true);
    buttonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    formLayout->addWidget(buttonBox);
    connect(buttonBox, SIGNAL(accepted()), &dialog, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), &dialog, SLOT(reject()));

    dialog.setLayout(formLayout);
    if(dialog.exec() == QDialog::Accepted)
    {
        int BusinessType = TypeCombo->currentData().toInt();
        QString command = QString::asprintf("RiskID:%s,Account:%s,Ticker:%s,EngineID:%d,BusinessType:%d,LongVolume:%d,ShortVolume:%d,LongLimit:%d,ShortLimit:%d,ExLowerLimit:%d,ExUpperLimit:%d,Trader:%s",                                                
                                            RiskIDEdit->text().toStdString().c_str(), AccountEdit->text().toStdString().c_str(), TickerEdit->text().toStdString().c_str(), 
                                            EngineIDEdit->text().toInt(), BusinessType, LongVolumeEdit->text().toInt(), ShortVolumeEdit->text().toInt(), 
                                            LongLimitEdit->text().toInt(), ShortLimitEdit->text().toInt(), ExLowerLimitEdit->text().toInt(), ExUpperLimitEdit->text().toInt(),
                                            TraderEdit->text().toStdString().c_str());   
        if(QMessageBox::Yes == QMessageBox::question(this, "Send Position Limit Command", command))
        {
            Message::PackMessage message;            
            message.MessageType = Message::EMessageType::ECommand;            
            message.Command.CmdType = Message::ECommandType::EUPDATE_RISK_POSITION_LIMIT;            
            strncpy(message.Command.Colo, m_RiskIDColoMap[RiskID].toStdString().c_str(), sizeof(message.Command.Colo));            
            strncpy(message.Command.Account, Account.toStdString().c_str(), sizeof(message.Command.Account));            
            strncpy(message.Command.Command, command.toStdString().c_str(), sizeof(message.Command.Command));            
            HPPackClient::SendData(reinterpret_cast<unsigned char *>(&message), sizeof(message));            
            FMTLOG(fmtlog::INF, "RiskJudgeWidget::OnUpdatePositionLimit Colo:{} Account:{} Command:{}",                     
                    message.Command.Colo, message.Command.Account, message.Command.Command);
        }         
    }
}

void RiskJudgeWidget::OnUpdateAccountLocked(const QModelIndex &index)
{
    int row = index.row();
    QString RiskID = m_AccountLockedProxyModel->index(row, 0).data().toString();
    QString Account = m_AccountLockedProxyModel->index(row, 1).data().toString();
    QString Ticker = m_AccountLockedProxyModel->index(row, 2).data().toString();
    QString Type = m_AccountLockedProxyModel->index(row, 3).data().toString();
    QString LockSide = m_AccountLockedProxyModel->index(row, 4).data().toString();
    QString Trader = m_AccountLockedProxyModel->index(row, 5).data().toString();

    QDialog dialog;
    dialog.setWindowTitle("Send Account Locked Command");
    dialog.setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint | Qt::WindowStaysOnTopHint);
    dialog.setFixedSize(400, 500);

    QFormLayout* formLayout = new QFormLayout;
    formLayout->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);

    QLineEdit* RiskIDEdit = new QLineEdit;
    RiskIDEdit->setText(RiskID);
    formLayout->addRow("RiskID: ", RiskIDEdit);
    
    QLineEdit* AccountEdit = new QLineEdit;
    AccountEdit->setText(Account);
    formLayout->addRow("Account: ", AccountEdit);

    QLineEdit* TickerEdit = new QLineEdit;
    TickerEdit->setText(Ticker);
    formLayout->addRow("Ticker: ", TickerEdit);

    QComboBox* TypeCombo = new QComboBox;
    TypeCombo->addItem("股票", Message::EBusinessType::ESTOCK);
    TypeCombo->addItem("信用", Message::EBusinessType::ECREDIT);
    TypeCombo->addItem("期货", Message::EBusinessType::EFUTURE);
    TypeCombo->addItem("现货", Message::EBusinessType::ESPOT);
    TypeCombo->setCurrentText(Type);
    formLayout->addRow("Type: ", TypeCombo);

    QComboBox* LockSideCombo = new QComboBox;
    LockSideCombo->addItem("解锁账户", Message::EAccountLockSide::EUNLOCK);
    LockSideCombo->addItem("禁止账户买卖", Message::EAccountLockSide::ELOCK_ACCOUNT);
    LockSideCombo->addItem("禁止开仓", Message::EAccountLockSide::ELOCK_OPEN);
    LockSideCombo->addItem("禁止平仓", Message::EAccountLockSide::ELOCK_CLOSE);
    LockSideCombo->addItem("禁止买入", Message::EAccountLockSide::ELOCK_BUY);
    LockSideCombo->addItem("禁止买入开仓", Message::EAccountLockSide::ELOCK_BUY_OPEN);
    LockSideCombo->addItem("禁止买入平仓", Message::EAccountLockSide::ELOCK_BUY_CLOSE);
    LockSideCombo->addItem("禁止买入平今", Message::EAccountLockSide::ELOCK_BUY_CLOSE_TODAY);
    LockSideCombo->addItem("禁止买入平昨", Message::EAccountLockSide::ELOCK_BUY_CLOSE_YESTODAY);
    LockSideCombo->addItem("禁止卖出", Message::EAccountLockSide::ELOCK_SELL);
    LockSideCombo->addItem("禁止卖出开仓", Message::EAccountLockSide::ELOCK_SELL_OPEN);
    LockSideCombo->addItem("禁止卖出平仓", Message::EAccountLockSide::ELOCK_SELL_CLOSE);
    LockSideCombo->addItem("禁止卖出平今", Message::EAccountLockSide::ELOCK_SELL_CLOSE_TODAY);
    LockSideCombo->addItem("禁止卖出平昨", Message::EAccountLockSide::ELOCK_SELL_CLOSE_YESTODAY);
    LockSideCombo->setCurrentText(LockSide);
    formLayout->addRow("LockSide: ", LockSideCombo);

    QLineEdit* TraderEdit = new QLineEdit;
    TraderEdit->setText(Trader);
    formLayout->addRow("Trader: ", TraderEdit);

    QDialogButtonBox* buttonBox = new QDialogButtonBox;
    buttonBox->setCenterButtons(true);
    buttonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    formLayout->addWidget(buttonBox);
    connect(buttonBox, SIGNAL(accepted()), &dialog, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), &dialog, SLOT(reject()));

    dialog.setLayout(formLayout);
    if(dialog.exec() == QDialog::Accepted)
    {
        int BusinessType = TypeCombo->currentData().toInt();
        int lockside = LockSideCombo->currentData().toInt();

        QString command = QString::asprintf("RiskID:%s,Account:%s,Ticker:%s,BusinessType:%d,LockSide:%d,Trader:%s",                                                
                                            RiskIDEdit->text().toStdString().c_str(), AccountEdit->text().toStdString().c_str(), TickerEdit->text().toStdString().c_str(), 
                                            BusinessType, lockside, TraderEdit->text().toStdString().c_str());            
        if(QMessageBox::Yes == QMessageBox::question(this, "Send Account Locked Command", command))
        {
            Message::PackMessage message;            
            message.MessageType = Message::EMessageType::ECommand;            
            message.Command.CmdType = Message::ECommandType::EUPDATE_RISK_ACCOUNT_LOCKED;            
            strncpy(message.Command.Colo, m_RiskIDColoMap[RiskID].toStdString().c_str(), sizeof(message.Command.Colo));            
            strncpy(message.Command.Account, Account.toStdString().c_str(), sizeof(message.Command.Account));            
            strncpy(message.Command.Command, command.toStdString().c_str(), sizeof(message.Command.Command));            
            HPPackClient::SendData(reinterpret_cast<unsigned char *>(&message), sizeof(message));            
            FMTLOG(fmtlog::INF, "RiskJudgeWidget::OnUpdatePositionLimit Colo:{} Account:{} Command:{}",                     
                    message.Command.Colo, message.Command.Account, message.Command.Command);
        }
    }
}

void RiskJudgeWidget::InitFilterWidget()
{
    m_FilterWidget = new FinTechUI::FilterWidget;
    QStringList filter;
    filter << "RiskID" <<  "Account" << "Ticker";
    m_FilterWidget->SetHeaderLabels(filter);
    m_FilterWidget->SetColumnWidth("RiskID", 70);
    m_FilterWidget->SetColumnWidth("Account", 100);
    m_FilterWidget->SetColumnWidth("Ticker", 70);
}

void RiskJudgeWidget::InitRiskLimitTableView()
{
    QStringList headers;
    headers << "RiskID" << "Account" << "Ticker" << "Type" << "FlowLimit" << "CancelCount" << "CancelLimit" << "OrderCount" << "OrderLimit" << "OrderCancelLimit" << "Trader" << "UpdateTime";
    m_RiskLimitTableView = new QTableView;
    m_RiskLimitTableView->setSortingEnabled(true);
    m_RiskLimitTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_RiskLimitTableView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_RiskLimitTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_RiskLimitTableView->verticalHeader()->hide();
    m_RiskLimitTableView->horizontalHeader()->setStretchLastSection(true);

    m_RiskLimitTableModel = new FinTechUI::XTableModel;
    m_RiskLimitTableModel->setObjectName("RiskLimitTable");
    m_RiskLimitTableModel->setHeaderLabels(headers);
    m_RiskLimitProxyModel = new FinTechUI::XSortFilterProxyModel;
    m_RiskLimitProxyModel->setSourceModel(m_RiskLimitTableModel);
    m_RiskLimitTableView->setModel(m_RiskLimitProxyModel);
    m_RiskLimitProxyModel->setDynamicSortFilter(true);
    m_RiskLimitTableView->sortByColumn(headers.size() - 1, Qt::DescendingOrder);

    int column = 0;
    m_RiskLimitTableView->setColumnWidth(column++, 70);
    m_RiskLimitTableView->setColumnWidth(column++, 100);
    m_RiskLimitTableView->setColumnWidth(column++, 70);
    m_RiskLimitTableView->setColumnWidth(column++, 70);
    m_RiskLimitTableView->setColumnWidth(column++, 70);
    m_RiskLimitTableView->setColumnWidth(column++, 70);
    m_RiskLimitTableView->setColumnWidth(column++, 70);
    m_RiskLimitTableView->setColumnWidth(column++, 70);
    m_RiskLimitTableView->setColumnWidth(column++, 70);
    m_RiskLimitTableView->setColumnWidth(column++, 100);
    m_RiskLimitTableView->setColumnWidth(column++, 80);
    m_RiskLimitTableView->setColumnWidth(column++, 120);
}

void RiskJudgeWidget::InitPositionLimitTableView()
{
    QStringList headers;
    headers << "RiskID" << "Account" << "Ticker" << "EngineID" << "Type" << "LongVolume" << "ShortVolume" << "LongLimit" << "ShortLimit" << "ExLowerLimit" << "ExUpperLimit" << "Trader" << "UpdateTime";
    m_PositionLimitTableView = new QTableView;
    m_PositionLimitTableView->setSortingEnabled(true);
    m_PositionLimitTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_PositionLimitTableView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_PositionLimitTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_PositionLimitTableView->verticalHeader()->hide();
    m_PositionLimitTableView->horizontalHeader()->setStretchLastSection(true);

    m_PositionLimitTableModel = new FinTechUI::XTableModel;
    m_PositionLimitTableModel->setObjectName("PositionLimitTable");
    m_PositionLimitTableModel->setHeaderLabels(headers);
    m_PositionLimitProxyModel = new FinTechUI::XSortFilterProxyModel;
    m_PositionLimitProxyModel->setSourceModel(m_PositionLimitTableModel);
    m_PositionLimitTableView->setModel(m_PositionLimitProxyModel);
    m_PositionLimitProxyModel->setDynamicSortFilter(true);
    m_PositionLimitTableView->sortByColumn(headers.size() - 1, Qt::DescendingOrder);

    int column = 0;
    m_PositionLimitTableView->setColumnWidth(column++, 70);
    m_PositionLimitTableView->setColumnWidth(column++, 100);
    m_PositionLimitTableView->setColumnWidth(column++, 70);
    m_PositionLimitTableView->setColumnWidth(column++, 70);
    m_PositionLimitTableView->setColumnWidth(column++, 70);
    m_PositionLimitTableView->setColumnWidth(column++, 70);
    m_PositionLimitTableView->setColumnWidth(column++, 75);
    m_PositionLimitTableView->setColumnWidth(column++, 75);
    m_PositionLimitTableView->setColumnWidth(column++, 75);
    m_PositionLimitTableView->setColumnWidth(column++, 75);
    m_PositionLimitTableView->setColumnWidth(column++, 75);
    m_PositionLimitTableView->setColumnWidth(column++, 75);
    m_PositionLimitTableView->setColumnWidth(column++, 120);
}

void RiskJudgeWidget::InitAccountLockedTableView()
{
    QStringList headers;
    headers << "RiskID" << "Account" << "Ticker" << "Type" << "LockSide" << "Trader" << "UpdateTime";
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
    m_AccountLockedProxyModel->setDynamicSortFilter(true);
    m_AccountLockedTableView->sortByColumn(headers.size() - 1, Qt::DescendingOrder);

    int column = 0;
    m_AccountLockedTableView->setColumnWidth(column++, 70);
    m_AccountLockedTableView->setColumnWidth(column++, 100);
    m_AccountLockedTableView->setColumnWidth(column++, 70);
    m_AccountLockedTableView->setColumnWidth(column++, 70);
    m_AccountLockedTableView->setColumnWidth(column++, 100);
    m_AccountLockedTableView->setColumnWidth(column++, 70);
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
    FMTLOG(fmtlog::INF, "RiskJudgeWidget::HandleRiskReport ReportType:{} Colo:{} Broker:{} Product:{} Account:{} Ticker:{} RiskID:{}", 
            report.RiskReport.ReportType, report.RiskReport.Colo, report.RiskReport.Broker, report.RiskReport.Product,
            report.RiskReport.Account, report.RiskReport.Ticker, report.RiskReport.RiskID);
    switch(report.RiskReport.ReportType)
    {
    case Message::ERiskReportType::ERISK_LIMIT:
        UpdateRiskLimitTable(report);
        break;
    case Message::ERiskReportType::ERISK_EVENTLOG:
        UpdateRiskEventTable(report);
        break;
    case Message::ERiskReportType::ERISK_POSITION_LIMIT:
        UpdatePositionLimitTable(report);
        break;
    case Message::ERiskReportType::ERISK_ACCOUNT_LOCKED:
        UpdateAccountLockedTable(report);
        break;
    }

    if(!m_RiskIDColoMap.contains(report.RiskReport.RiskID))
    {
        m_RiskIDColoMap[report.RiskReport.RiskID] = report.RiskReport.Colo;
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

void RiskJudgeWidget::UpdateRiskLimitTable(const Message::PackMessage& report)
{
    QString RiskID = report.RiskReport.RiskID;
    QString Account = report.RiskReport.Account;
    QString Ticker = report.RiskReport.Ticker;

    QString Key = RiskID + ":" + Account + ":" + Ticker;
    auto it = m_RiskLimitReportMap.find(Key);
    if(it != m_RiskLimitReportMap.end())
    {
        FinTechUI::XTableModelRow* ModelRow = m_RiskLimitReportMap[Key];
        (*ModelRow)[4]->setText(report.RiskReport.FlowLimit);
        (*ModelRow)[5]->setText(report.RiskReport.CancelCount);
        (*ModelRow)[6]->setText(report.RiskReport.CancelLimit);
        (*ModelRow)[7]->setText(report.RiskReport.OrderCount);
        (*ModelRow)[8]->setText(report.RiskReport.OrderLimit);
        (*ModelRow)[9]->setText(report.RiskReport.OrderCancelLimit);
         (*ModelRow)[10]->setText(report.RiskReport.Trader);
        (*ModelRow)[11]->setText(report.RiskReport.UpdateTime + 11);

        m_RiskLimitTableModel->updateRow(ModelRow);
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
        FinTechUI::XTableModelItem* TypeItem = new FinTechUI::XTableModelItem(GetBusinessType(report.RiskReport.BusinessType));
        ModelRow->push_back(TypeItem);
        FinTechUI::XTableModelItem* FlowLimitItem = new FinTechUI::XTableModelItem(report.RiskReport.FlowLimit);
        ModelRow->push_back(FlowLimitItem);
        FinTechUI::XTableModelItem* CancelCountItem = new FinTechUI::XTableModelItem(report.RiskReport.CancelCount);
        ModelRow->push_back(CancelCountItem);
        FinTechUI::XTableModelItem* CancelLimitItem = new FinTechUI::XTableModelItem(report.RiskReport.CancelLimit);
        ModelRow->push_back(CancelLimitItem);
        FinTechUI::XTableModelItem* OrderCountItem = new FinTechUI::XTableModelItem(report.RiskReport.OrderCount);
        ModelRow->push_back(OrderCountItem);
        FinTechUI::XTableModelItem* OrderLimitItem = new FinTechUI::XTableModelItem(report.RiskReport.OrderLimit);
        ModelRow->push_back(OrderLimitItem);
        FinTechUI::XTableModelItem* OrderCancelLimitItem = new FinTechUI::XTableModelItem(report.RiskReport.OrderCancelLimit);
        ModelRow->push_back(OrderCancelLimitItem);
        FinTechUI::XTableModelItem* TraderItem = new FinTechUI::XTableModelItem(report.RiskReport.Trader);
        ModelRow->push_back(TraderItem);
        FinTechUI::XTableModelItem* UpdateTimeItem = new FinTechUI::XTableModelItem(report.RiskReport.UpdateTime + 11);
        ModelRow->push_back(UpdateTimeItem);

        FinTechUI::XTableModel::setRowBackgroundColor(ModelRow, GetRiskReportColor(report.RiskReport));
        m_RiskLimitTableModel->appendRow(ModelRow);
        m_RiskLimitReportMap[Key] = ModelRow;
    }
    
    FMTLOG(fmtlog::INF, "RiskJudgeWidget::UpdateRiskLimitTable RiskID:{} Account:{} Ticker:{} BusinessType:{} FlowLimit:{} CancelCount:{} CancelLimit:{} OrderCount:{} OrderLimit:{} OrderCancelLimit:{} Trader:{}", 
            report.RiskReport.RiskID, report.RiskReport.Account, report.RiskReport.Ticker, report.RiskReport.BusinessType, 
            report.RiskReport.FlowLimit, report.RiskReport.CancelCount, report.RiskReport.CancelLimit, report.RiskReport.OrderCount, 
            report.RiskReport.OrderLimit, report.RiskReport.OrderCancelLimit, report.RiskReport.Trader);
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
    // QMessageBox::warning(this, "Risk Warning", report.RiskReport.Event);
    FMTLOG(fmtlog::INF, "RiskJudgeWidget::UpdateRiskEventTable RiskID:{} Account:{} Ticker:{} Event:{}", 
            report.RiskReport.RiskID, report.RiskReport.Account, report.RiskReport.Ticker, report.RiskReport.Event);
}

void RiskJudgeWidget::UpdatePositionLimitTable(const Message::PackMessage& report)
{
    QString RiskID = report.RiskReport.RiskID;
    QString Account = report.RiskReport.Account;
    QString Ticker = report.RiskReport.Ticker;
    QString EngineID = QString::number(report.RiskReport.EngineID);

    QString Key = RiskID + ":" + Account + ":" + Ticker + ":" + EngineID;
    auto it = m_PositionLimitReportMap.find(Key);
    if(it != m_PositionLimitReportMap.end())
    {
        FinTechUI::XTableModelRow* ModelRow = m_PositionLimitReportMap[Key];
        (*ModelRow)[5]->setText(report.RiskReport.LongVolume);
        (*ModelRow)[6]->setText(report.RiskReport.ShortVolume);
        (*ModelRow)[7]->setText(report.RiskReport.LongLimit);
        (*ModelRow)[8]->setText(report.RiskReport.ShortLimit);
        (*ModelRow)[9]->setText(report.RiskReport.ExposureLowerLimit);
        (*ModelRow)[10]->setText(report.RiskReport.ExposureUpperLimit);
        (*ModelRow)[11]->setText(report.RiskReport.Trader);
        (*ModelRow)[12]->setText(report.RiskReport.UpdateTime + 11);

        m_PositionLimitTableModel->updateRow(ModelRow);
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
        FinTechUI::XTableModelItem* EngineIDItem = new FinTechUI::XTableModelItem(report.RiskReport.EngineID);
        ModelRow->push_back(EngineIDItem);
        FinTechUI::XTableModelItem* TypeItem = new FinTechUI::XTableModelItem(GetBusinessType(report.RiskReport.BusinessType));
        ModelRow->push_back(TypeItem);
        FinTechUI::XTableModelItem* LongVolumeItem = new FinTechUI::XTableModelItem(report.RiskReport.LongVolume);
        ModelRow->push_back(LongVolumeItem);
        FinTechUI::XTableModelItem* ShortVolumeItem = new FinTechUI::XTableModelItem(report.RiskReport.ShortVolume);
        ModelRow->push_back(ShortVolumeItem);
        FinTechUI::XTableModelItem* LongLimitItem = new FinTechUI::XTableModelItem(report.RiskReport.LongLimit);
        ModelRow->push_back(LongLimitItem);
        FinTechUI::XTableModelItem* ShortLimitItem = new FinTechUI::XTableModelItem(report.RiskReport.ShortLimit);
        ModelRow->push_back(ShortLimitItem);
        FinTechUI::XTableModelItem* ExposureLowerLimitItem = new FinTechUI::XTableModelItem(report.RiskReport.ExposureLowerLimit);
        ModelRow->push_back(ExposureLowerLimitItem);
        FinTechUI::XTableModelItem* ExposureUpperLimitItem = new FinTechUI::XTableModelItem(report.RiskReport.ExposureUpperLimit);
        ModelRow->push_back(ExposureUpperLimitItem);
        FinTechUI::XTableModelItem* TraderItem = new FinTechUI::XTableModelItem(report.RiskReport.Trader);
        ModelRow->push_back(TraderItem);
        FinTechUI::XTableModelItem* UpdateTimeItem = new FinTechUI::XTableModelItem(report.RiskReport.UpdateTime + 11);
        ModelRow->push_back(UpdateTimeItem);

        FinTechUI::XTableModel::setRowBackgroundColor(ModelRow, GetRiskReportColor(report.RiskReport));
        m_PositionLimitTableModel->appendRow(ModelRow);
        m_PositionLimitReportMap[Key] = ModelRow;
    }
    
    FMTLOG(fmtlog::INF, "RiskJudgeWidget::UpdatePositionLimitTable RiskID:{} Account:{} Ticker:{} EngineID:{} BusinessType:{} LongVolume:{} ShortVolume:{} LongLimit:{} ShortLimit:{} ExposureLowerLimit:{} ExposureUpperLimit:{} Trader:{}", 
            report.RiskReport.RiskID, report.RiskReport.Account, report.RiskReport.Ticker, report.RiskReport.EngineID, report.RiskReport.BusinessType,
            report.RiskReport.LongVolume, report.RiskReport.ShortVolume, report.RiskReport.LongLimit, report.RiskReport.ShortLimit, 
            report.RiskReport.ExposureLowerLimit, report.RiskReport.ExposureUpperLimit, report.RiskReport.Trader);
}

void RiskJudgeWidget::UpdateAccountLockedTable(const Message::PackMessage& report)
{
    QString RiskID = report.RiskReport.RiskID;
    QString Account = report.RiskReport.Account;
    QString Ticker = report.RiskReport.Ticker;

    QString Key = RiskID + ":" + Account + ":" + Ticker;
    auto it = m_AccountLockedReportMap.find(Key);
    if(it != m_AccountLockedReportMap.end())
    {
        FinTechUI::XTableModelRow* ModelRow = m_AccountLockedReportMap[Key];
        (*ModelRow)[4]->setText(GetLockSide(report.RiskReport.LockSide));
        (*ModelRow)[5]->setText(report.RiskReport.Trader);
        (*ModelRow)[6]->setText(report.RiskReport.UpdateTime);

        m_AccountLockedTableModel->updateRow(ModelRow);
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
        FinTechUI::XTableModelItem* TypeItem = new FinTechUI::XTableModelItem(GetBusinessType(report.RiskReport.BusinessType));
        ModelRow->push_back(TypeItem);
        FinTechUI::XTableModelItem* LockSideItem = new FinTechUI::XTableModelItem(GetLockSide(report.RiskReport.LockSide));
        ModelRow->push_back(LockSideItem);
        FinTechUI::XTableModelItem* TraderItem = new FinTechUI::XTableModelItem(report.RiskReport.Trader);
        ModelRow->push_back(TraderItem);
        FinTechUI::XTableModelItem* UpdateTimeItem = new FinTechUI::XTableModelItem(report.RiskReport.UpdateTime);
        ModelRow->push_back(UpdateTimeItem);

        FinTechUI::XTableModel::setRowBackgroundColor(ModelRow, GetRiskReportColor(report.RiskReport));
        m_AccountLockedTableModel->appendRow(ModelRow);
        m_AccountLockedReportMap[Key] = ModelRow;
    }
    
    FMTLOG(fmtlog::INF, "RiskJudgeWidget::UpdateAccountLockedTable RiskID:{} Account:{} Ticker:{} BusinessType:{} LockSide:{} Trader:{}", 
            report.RiskReport.RiskID, report.RiskReport.Account, report.RiskReport.Ticker, report.RiskReport.BusinessType,
            report.RiskReport.LockSide, report.RiskReport.Trader);
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
