#include "OrderManagerWidget.h"
extern Utils::Logger *gLogger;

OrderManagerWidget::OrderManagerWidget(QWidget *parent) : FinTechUI::TabPageWidget(parent)
{   
    InitFilterWidget();
    InitControlPannel();
    InitStockTickerPositionTable();
    InitFutureTickerPositionTable();
    InitHangingOrderTable();
    InitHistoryOrderTable();
    InitAccountFundTable();

    m_LeftWidget = new QWidget;
    QVBoxLayout* leftLayout = new QVBoxLayout;
    leftLayout->setContentsMargins(0, 0, 0, 0);
    leftLayout->setSpacing(10);
    leftLayout->addWidget(m_FilterWidget);
    leftLayout->addWidget(m_ControlPannelWidget);
    leftLayout->setStretch(0, 9);
    leftLayout->setStretch(1, 1);
    leftLayout->addStretch(1);
    m_LeftWidget->setLayout(leftLayout);
    m_ControlPannelWidget->setMaximumWidth(m_FilterWidget->maximumWidth());
    m_LeftWidget->setMinimumWidth(200);
    m_LeftWidget->setMaximumWidth(m_FilterWidget->width());

    m_MiddleSplitter = new QSplitter(Qt::Vertical);
    m_MiddleSplitter->setHandleWidth(0);
    m_MiddleSplitter->addWidget(m_StockTickerPositionTableView);
    m_MiddleSplitter->addWidget(m_FutureTickerPositionTableView);
    m_MiddleSplitter->addWidget(m_HangingOrderTableView);
    m_MiddleSplitter->addWidget(m_HistoryOrderTableView);
    m_MiddleSplitter->setStretchFactor(0, 2);
    m_MiddleSplitter->setStretchFactor(1, 2);
    m_MiddleSplitter->setStretchFactor(2, 2);
    m_MiddleSplitter->setStretchFactor(3, 4);
    m_MiddleSplitter->setCollapsible(0, true);
    m_MiddleSplitter->setCollapsible(1, true);
    
    m_Splitter = new QSplitter(Qt::Horizontal);
    m_Splitter->addWidget(m_LeftWidget);
    m_Splitter->addWidget(m_MiddleSplitter);
    m_Splitter->addWidget(m_AccountFundTableView);
    m_Splitter->setStretchFactor(0, 3);
    m_Splitter->setStretchFactor(1, 18);
    m_Splitter->setStretchFactor(2, 3);
    m_Splitter->setCollapsible(0, true);
    m_Splitter->setCollapsible(2, true);

    QHBoxLayout* layout = new QHBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_Splitter);
    setLayout(layout);

    m_FlushTimer = startTimer(200, Qt::PreciseTimer);
    connect(m_HangingOrderTableView, SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT(OnEndOrder(const QModelIndex&)));
    connect(m_FilterWidget, &FinTechUI::FilterWidget::FilterChanged, this, &OrderManagerWidget::OnFilterTable, Qt::UniqueConnection);
}

void OrderManagerWidget::OnReceivedExecutionReport(const QList<Message::PackMessage>& messages)
{
    m_ExecutionReportQueue.append(messages);
}

void OrderManagerWidget::OnSendOrder()
{
    QDialog dialog;
    dialog.setWindowTitle("Send Order");
    dialog.setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint | Qt::WindowStaysOnTopHint);
    dialog.setFixedSize(500, 400);

    QFormLayout* formLayout = new QFormLayout;
    formLayout->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);
    QStringList Products = m_ProductAccountSetMap.keys();
    Products.sort();
    QComboBox* productCombo = new QComboBox;
    productCombo->setFixedWidth(120);
    productCombo->addItems(Products);
    formLayout->addRow("Product: ", productCombo);

    QComboBox* accountCombo = new QComboBox;
    QStringList accounts = m_ProductAccountSetMap[productCombo->currentText()];
    accounts.sort();
    accountCombo->addItems(accounts);
    formLayout->addRow("Account: ", accountCombo);

    connect(productCombo, &QComboBox::currentTextChanged, accountCombo,
            [ & ](const QString & text)
    {
        accounts = m_ProductAccountSetMap[text];
        accounts.sort();
        accountCombo->clear();
        accountCombo->addItems(accounts);
    });

    QComboBox* TickerCombo = new QComboBox;
    TickerCombo->setEditable(true);
    TickerCombo->setFixedWidth(120);
    formLayout->addRow("Ticker: ", TickerCombo);
    connect(accountCombo, &QComboBox::currentTextChanged,
            [ & ](const QString & text)
    {
        QStringList Tickers = m_AccountTickerSetMap[accountCombo->currentText()];
        Tickers.sort();
        TickerCombo->clear();
        TickerCombo->addItems(Tickers);
    });

    QComboBox* ExchangeCombo = new QComboBox;
    ExchangeCombo->addItems(QStringList() << "SH" << "SZ" << "CFFEX" << "DCE" << "CZCE" << "SHFE" << "INE");
    formLayout->addRow("Exchange: ", ExchangeCombo);

    QComboBox* orderTypeCombo = new QComboBox;
    orderTypeCombo->addItems(QStringList() << "FAK" << "FOK" << "LIMIT");
    formLayout->addRow("OrderType: ", orderTypeCombo);

    QComboBox* directionCombo = new QComboBox;
    directionCombo->addItems(QStringList() << "Buy" << "Sell" << "ReverseRepo" << "Subscription" << "Allotment" 
                                           << "CollateralTransferIn" << "CollateralTransferOut" << "MarginBuy" << "RepayMarginBySell"
                                           << "ShortSell" << "RepayStockByBuy" << "RepayStockDirect");
    formLayout->addRow("Direction: ", directionCombo);
    QComboBox* offsetCombo = new QComboBox;
    offsetCombo->addItems(QStringList() << "None" << "Open" << "Close" << "CloseToday" << "CloseYestoday");
    formLayout->addRow("Offset: ", offsetCombo);
    QComboBox* riskCheckCombo = new QComboBox;
    riskCheckCombo->addItems(QStringList() << "Check" << "NoCheck");
    formLayout->addRow("RiskCheck: ", riskCheckCombo);
    QComboBox* engineCombo = new QComboBox;
    engineCombo->addItems(QStringList() << "None" << "TraderOrder");
    formLayout->addRow("Engine: ", engineCombo);
    QLineEdit* priceEdit = new QLineEdit;
    formLayout->addRow("Price: ", priceEdit);
    QLineEdit* volumeEdit = new QLineEdit;
    formLayout->addRow("Volume: ", volumeEdit);
    QLineEdit* countEdit = new QLineEdit;
    countEdit->setText("1");
    formLayout->addRow("Count: ", countEdit);
    QDialogButtonBox* buttonBox = new QDialogButtonBox;
    buttonBox->setCenterButtons(true);
    buttonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    formLayout->addWidget(buttonBox);
    connect(buttonBox, SIGNAL(accepted()), &dialog, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), &dialog, SLOT(reject()));

    dialog.setLayout(formLayout);
    if(dialog.exec() == QDialog::Accepted)
    {
        Message::PackMessage message;
        memset(&message, 0, sizeof (message));
        message.MessageType = Message::EMessageType::EOrderRequest;
        QString Product = productCombo->currentText();
        QString Account = accountCombo->currentText();
        QString Ticker = TickerCombo->currentText();
        QString Exchange = ExchangeCombo->currentText();
        memcpy(message.OrderRequest.Colo, m_AccountColoMap[Account].toStdString().c_str(), sizeof(message.OrderRequest.Colo));
        memcpy(message.OrderRequest.Product, Product.toStdString().c_str(), sizeof(message.OrderRequest.Product));
        memcpy(message.OrderRequest.Account, Account.toStdString().c_str(), sizeof(message.OrderRequest.Account));
        memcpy(message.OrderRequest.Ticker, Ticker.toStdString().c_str(), sizeof(message.OrderRequest.Ticker));
        memcpy(message.OrderRequest.ExchangeID, Exchange.toStdString().c_str(), sizeof(message.OrderRequest.ExchangeID));
        if(orderTypeCombo->currentText() == "FAK")
        {
            message.OrderRequest.OrderType = Message::EOrderType::EFAK;
        }
        else if(orderTypeCombo->currentText() == "FOK")
        {
            message.OrderRequest.OrderType = Message::EOrderType::EFOK;
        }
        else if(orderTypeCombo->currentText() == "LIMIT")
        {
            message.OrderRequest.OrderType = Message::EOrderType::ELIMIT;
        }

        if(directionCombo->currentText() == "Buy")
        {
            message.OrderRequest.Direction = Message::EOrderDirection::EBUY;
        }
        else if(directionCombo->currentText() == "Sell")
        {
            message.OrderRequest.Direction = Message::EOrderDirection::ESELL;
        }
        else if(directionCombo->currentText() == "ReverseRepo")
        {
            message.OrderRequest.Direction = Message::EOrderDirection::EREVERSE_REPO;
        }
        else if(directionCombo->currentText() == "Subscription")
        {
            message.OrderRequest.Direction = Message::EOrderDirection::ESUBSCRIPTION;
        }
        else if(directionCombo->currentText() == "Allotment")
        {
            message.OrderRequest.Direction = Message::EOrderDirection::EALLOTMENT;
        }
        else if(directionCombo->currentText() == "CollateralTransferIn")
        {
            message.OrderRequest.Direction = Message::EOrderDirection::ECOLLATERAL_TRANSFER_IN;
        }
        else if(directionCombo->currentText() == "CollateralTransferOut")
        {
            message.OrderRequest.Direction = Message::EOrderDirection::ECOLLATERAL_TRANSFER_OUT;
        }
        else if(directionCombo->currentText() == "MarginBuy")
        {
            message.OrderRequest.Direction = Message::EOrderDirection::EMARGIN_BUY;
        }
        else if(directionCombo->currentText() == "RepayMarginBySell")
        {
            message.OrderRequest.Direction = Message::EOrderDirection::EREPAY_MARGIN_BY_SELL;
        }
        else if(directionCombo->currentText() == "ShortSell")
        {
            message.OrderRequest.Direction = Message::EOrderDirection::ESHORT_SELL;
        }
        else if(directionCombo->currentText() == "RepayStockByBuy")
        {
            message.OrderRequest.Direction = Message::EOrderDirection::EREPAY_STOCK_BY_BUY;
        }
        else if(directionCombo->currentText() == "RepayStockDirect")
        {
            message.OrderRequest.Direction = Message::EOrderDirection::EREPAY_STOCK_DIRECT;
        }

        if(offsetCombo->currentText() == "Open")
        {
            message.OrderRequest.Offset = Message::EOrderOffset::EOPEN;
        }
        else if(offsetCombo->currentText() == "Close")
        {
            message.OrderRequest.Offset = Message::EOrderOffset::ECLOSE;
        }
        else if(offsetCombo->currentText() == "CloseToday")
        {
            message.OrderRequest.Offset = Message::EOrderOffset::ECLOSE_TODAY;
        }
        else if(offsetCombo->currentText() == "CloseYestoday")
        {
            message.OrderRequest.Offset = Message::EOrderOffset::ECLOSE_YESTODAY;
        }
        
        if(riskCheckCombo->currentText() == "Check")
        {
            message.OrderRequest.RiskStatus = Message::ERiskStatusType::EPREPARE_CHECKED;
        }
        else if(riskCheckCombo->currentText() == "NoCheck")
        {
            message.OrderRequest.RiskStatus = Message::ERiskStatusType::ENOCHECKED;
        }
        if(engineCombo->currentText() == "TraderOder")
        {
            message.OrderRequest.EngineID = Message::EEngineType::ETRADER_ORDER;
        }
        message.OrderRequest.Price = priceEdit->text().toDouble();
        message.OrderRequest.Volume = volumeEdit->text().toInt();
        QString currentTime = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz") + "000";
        strncpy(message.OrderRequest.SendTime, currentTime.toStdString().c_str(), sizeof(message.OrderRequest.SendTime));
        Utils::gLogger->Log->info("OrderManagerWidget::OnSendOrder Colo:{} Account:{} Ticker:{} Price:{} Volume:{}", 
                                    message.OrderRequest.Colo, message.OrderRequest.Account, message.OrderRequest.Ticker, message.OrderRequest.Price, message.OrderRequest.Volume);
        int n = countEdit->text().toInt();
        for (int i = 0; i < n; i++)
        {
            HPPackClient::SendData(reinterpret_cast<unsigned char *>(&message), sizeof(message));
        }
    }
}

void OrderManagerWidget::OnCancelOrder()
{
    CancelOrderDialog("", "");
}

void OrderManagerWidget::OnEndOrder(const QModelIndex &index)
{
    int row = index.row();
    QString Account = m_HangingOrderProxyModel->index(row, 3).data().toString();
    QString OrderRef = m_HangingOrderProxyModel->index(row, 12).data().toString();
    CancelOrderDialog(Account, OrderRef);
}

void OrderManagerWidget::OnFilterTable(const QVector<QStringList>& filter)
{
    QStringList ColoFilter = filter.at(0);
    QStringList ProductFilter = filter.at(1);
    QStringList AccountFilter = filter.at(2);
    QStringList TickerFilter = filter.at(3);

    QMap<int, QStringList> FilterMap;
    FilterMap[0] = ColoFilter;
    FilterMap[2] = ProductFilter;
    FilterMap[3] = AccountFilter;
    FilterMap[4] = TickerFilter;

    m_StockTickerPositionProxyModel->setRowFilter(FilterMap);
    m_StockTickerPositionProxyModel->resetFilter();

    m_FutureTickerPositionProxyModel->setRowFilter(FilterMap);
    m_FutureTickerPositionProxyModel->resetFilter();

    m_HangingOrderProxyModel->setRowFilter(FilterMap);
    m_HangingOrderProxyModel->resetFilter();

    m_HistoryOrderProxyModel->setRowFilter(FilterMap);
    m_HistoryOrderProxyModel->resetFilter();

    FilterMap.clear();
    FilterMap[0] = AccountFilter;
    m_AccountFundProxyModel->setRowFilter(FilterMap);
    m_AccountFundProxyModel->resetFilter();
}

void OrderManagerWidget::OnExportOrderTable()
{
    QString path = QCoreApplication::applicationDirPath();
    ExportTable(m_HistoryOrderTableModel, path);
}

void OrderManagerWidget::OnTransferFund()
{
    QDialog dialog;
    dialog.setWindowTitle("Transfer Fund");
    dialog.setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint | Qt::WindowStaysOnTopHint);
    dialog.setFixedSize(400, 300);

    QFormLayout* formLayout = new QFormLayout;
    formLayout->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);

    QComboBox* accountCombo = new QComboBox;
    QStringList accounts = m_AccountSet.toList();
    accounts.sort();
    accountCombo->addItems(accounts);
    formLayout->addRow("Account: ", accountCombo);

    QComboBox* directionCombo = new QComboBox;
    directionCombo->addItems(QStringList() << "In" << "Out");
    directionCombo->setCurrentText("In");
    formLayout->addRow("Direction: ", directionCombo);

    QLineEdit* AmountEdit = new QLineEdit;
    QIntValidator* AmountEditValidator = new QIntValidator();
    AmountEditValidator->setRange(0, 1000000000);
    AmountEdit->setValidator(AmountEditValidator);
    formLayout->addRow("Amount: ", AmountEdit);

    QDialogButtonBox* buttonBox = new QDialogButtonBox;
    buttonBox->setCenterButtons(true);
    buttonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    formLayout->addWidget(buttonBox);
    connect(buttonBox, SIGNAL(accepted()), &dialog, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), &dialog, SLOT(reject()));

    dialog.setLayout(formLayout);
    if(dialog.exec() == QDialog::Accepted)
    {
        Message::PackMessage message;
        memset(&message, 0, sizeof (message));
        message.MessageType = Message::EMessageType::ECommand;
        QString Account = accountCombo->currentText();
        QString Direction = directionCombo->currentText();
        if(directionCombo->currentText() == "In")
        {
            message.Command.CmdType = Message::ECommandType::ETRANSFER_FUND_IN;
        }
        else if(directionCombo->currentText() == "Out")
        {
            message.Command.CmdType = Message::ECommandType::ETRANSFER_FUND_OUT;
        }
        QString Colo = m_AccountColoMap[Account];
        strncpy(message.Command.Account, Account.toStdString().c_str(), sizeof(message.Command.Account));
        strncpy(message.Command.Colo, Colo.toStdString().c_str(), sizeof(message.Command.Colo));
        int Amount = AmountEdit->text().toInt();
        sprintf( message.Command.Command, "Amount:%d", Amount);
        Utils::gLogger->Log->info("OrderManagerWidget::OnTransferFund Colo:{} Account:{} CmdType:{} Command:{}", 
                                    message.Command.Colo, message.Command.Account, message.Command.CmdType, message.Command.Command);
        HPPackClient::SendData(reinterpret_cast<unsigned char *>(&message), sizeof(message));
    }
}

void OrderManagerWidget::OnRepayMargin()
{
    QDialog dialog;
    dialog.setWindowTitle("Repay Margin Direct");
    dialog.setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint | Qt::WindowStaysOnTopHint);
    dialog.setFixedSize(400, 300);

    QFormLayout* formLayout = new QFormLayout;
    formLayout->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);

    QComboBox* accountCombo = new QComboBox;
    QStringList accounts = m_AccountSet.toList();
    accounts.sort();
    accountCombo->addItems(accounts);
    formLayout->addRow("Account: ", accountCombo);

    QLineEdit* AmountEdit = new QLineEdit;
    QIntValidator* AmountEditValidator = new QIntValidator();
    AmountEditValidator->setRange(0, 1000000000);
    AmountEdit->setValidator(AmountEditValidator);
    formLayout->addRow("Amount: ", AmountEdit);

    QDialogButtonBox* buttonBox = new QDialogButtonBox;
    buttonBox->setCenterButtons(true);
    buttonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    formLayout->addWidget(buttonBox);
    connect(buttonBox, SIGNAL(accepted()), &dialog, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), &dialog, SLOT(reject()));

    dialog.setLayout(formLayout);
    if(dialog.exec() == QDialog::Accepted)
    {
        Message::PackMessage message;
        memset(&message, 0, sizeof (message));
        message.MessageType = Message::EMessageType::ECommand;
        QString Account = accountCombo->currentText();
        message.Command.CmdType = Message::ECommandType::EREPAY_MARGIN_DIRECT;
        QString Colo = m_AccountColoMap[Account];
        strncpy(message.Command.Account, Account.toStdString().c_str(), sizeof(message.Command.Account));
        strncpy(message.Command.Colo, Colo.toStdString().c_str(), sizeof(message.Command.Colo));
        int Amount = AmountEdit->text().toInt();
        sprintf( message.Command.Command, "Amount:%d", Amount);
        Utils::gLogger->Log->info("OrderManagerWidget::OnRepayMargin Colo:{} Account:{} Command:{}", 
                                    message.Command.Colo, message.Command.Account, message.Command.Command);
        HPPackClient::SendData(reinterpret_cast<unsigned char *>(&message), sizeof(message));
    } 
}

void OrderManagerWidget::InitFilterWidget()
{
    m_FilterWidget = new FinTechUI::FilterWidget;
    m_FilterWidget->SetHeaderLabels(QStringList() << "Colo" << "Product" << "Account" << "Ticker");
    m_FilterWidget->SetColumnWidth("Colo", 60);
    m_FilterWidget->SetColumnWidth("Product", 70);
    m_FilterWidget->SetColumnWidth("Account", 90);
    m_FilterWidget->SetColumnWidth("Ticker", 90);
    m_FilterWidget->setFixedWidth(60 + 70 + 90 + 90 + 35);
}

void OrderManagerWidget::InitControlPannel()
{
    m_ControlPannelWidget = new QWidget;
    QPushButton* sendOrderButton = new QPushButton("SendOrder");
    QPushButton* cancelOrderButton = new QPushButton("CancelOrder");
    QPushButton* exportOrderTableButton = new QPushButton("ExportOrder");
    QPushButton* transferFundButton = new QPushButton("TransferFund");
    QPushButton* repayMarginButton = new QPushButton("RepayMargin");
    QGridLayout* buttonLayout = new QGridLayout;
    buttonLayout->setContentsMargins(0, 0, 0, 0);
    buttonLayout->setHorizontalSpacing(20);
    buttonLayout->setVerticalSpacing(10);
    buttonLayout->addWidget(sendOrderButton, 0, 0);
    buttonLayout->addWidget(cancelOrderButton, 0, 1);
    buttonLayout->addWidget(exportOrderTableButton, 0, 2);
    buttonLayout->addWidget(transferFundButton, 1, 0);
    buttonLayout->addWidget(repayMarginButton, 1, 1);
    buttonLayout->setRowStretch(2, 1);
    m_ControlPannelWidget->setLayout(buttonLayout);

    connect(sendOrderButton, SIGNAL(clicked(bool)), this, SLOT(OnSendOrder()));
    connect(cancelOrderButton, SIGNAL(clicked(bool)), this, SLOT(OnCancelOrder()));
    connect(exportOrderTableButton, SIGNAL(clicked(bool)), this, SLOT(OnExportOrderTable()));
    connect(transferFundButton, SIGNAL(clicked(bool)), this, SLOT(OnTransferFund()));
    connect(repayMarginButton, SIGNAL(clicked(bool)), this, SLOT(OnRepayMargin()));
}

void OrderManagerWidget::InitStockTickerPositionTable()
{
    QStringList headerData;
    headerData << "Colo" << "Broker" << "Product" << "Account" << "Ticker" << "Exchange" << "LongYdPosition" << "LongPosition" 
               << "LongTdBuy" << "LongTdSell" << "MarginYdPosition" << "MarginPosition" << "MarginTdBuy" << "MarginTdSell"
               << "ShortYdPosition" << "ShortPosition" << "ShortTdSell" << "ShortTdBuy" << "ShortDirectRepaid" << "SpecialPositionAvl" << "UpdateTime";
    m_StockTickerPositionTableView = new FinTechUI::FrozenTableView(6);
    m_StockTickerPositionTableView->setObjectName("StockPositionTable");
    m_StockTickerPositionTableModel = new FinTechUI::XTableModel;
    m_StockTickerPositionTableModel->setObjectName("StockPositionTable");
    m_StockTickerPositionProxyModel = new FinTechUI::XSortFilterProxyModel;
    m_StockTickerPositionProxyModel->setSourceModel(m_StockTickerPositionTableModel);
    m_StockTickerPositionTableModel->setHeaderLabels(headerData);
    m_StockTickerPositionTableView->setModel(m_StockTickerPositionProxyModel);
    m_StockTickerPositionProxyModel->setDynamicSortFilter(true);
    m_StockTickerPositionTableView->sortByColumn(20, Qt::DescendingOrder);
    
    int column = 0;
    m_StockTickerPositionTableView->setColumnWidth(column++, 60);
    m_StockTickerPositionTableView->setColumnWidth(column++, 50);
    m_StockTickerPositionTableView->setColumnWidth(column++, 80);
    m_StockTickerPositionTableView->setColumnWidth(column++, 100);
    m_StockTickerPositionTableView->setColumnWidth(column++, 80);
    m_StockTickerPositionTableView->setColumnWidth(column++, 70);
    m_StockTickerPositionTableView->setColumnWidth(column++, 100);
    m_StockTickerPositionTableView->setColumnWidth(column++, 100);
    m_StockTickerPositionTableView->setColumnWidth(column++, 80);
    m_StockTickerPositionTableView->setColumnWidth(column++, 80);
    m_StockTickerPositionTableView->setColumnWidth(column++, 110);
    m_StockTickerPositionTableView->setColumnWidth(column++, 100);
    m_StockTickerPositionTableView->setColumnWidth(column++, 100);
    m_StockTickerPositionTableView->setColumnWidth(column++, 100);
    m_StockTickerPositionTableView->setColumnWidth(column++, 100);
    m_StockTickerPositionTableView->setColumnWidth(column++, 100);
    m_StockTickerPositionTableView->setColumnWidth(column++, 100);
    m_StockTickerPositionTableView->setColumnWidth(column++, 100);
    m_StockTickerPositionTableView->setColumnWidth(column++, 120);
    m_StockTickerPositionTableView->setColumnWidth(column++, 120);
    m_StockTickerPositionTableView->setColumnWidth(column++, 120);
}

void OrderManagerWidget::InitFutureTickerPositionTable()
{
    QStringList headerData;
    headerData << "Colo" << "Broker" << "Product" << "Account" << "Ticker" << "Exchange" << "LongTdVolume" << "LongYdVolume" 
               << "LongOpenVolume" << "LongOpenFrozen" << "LongCloseTdFrozen" << "LongCloseYdFrozen" << "ShortTdVolume" 
               << "ShortYdVolume" << "ShortOpenVolume" << "ShortOpenFrozen" << "ShortCloseTdFrozen" << "ShortCloseYdFrozen" << "UpdateTime";
    m_FutureTickerPositionTableView = new FinTechUI::FrozenTableView(6);
    m_FutureTickerPositionTableView->setObjectName("FuturePositionTable");
    m_FutureTickerPositionTableModel = new FinTechUI::XTableModel;
    m_FutureTickerPositionTableModel->setObjectName("FuturePositionTable");
    m_FutureTickerPositionProxyModel = new FinTechUI::XSortFilterProxyModel;
    m_FutureTickerPositionProxyModel->setSourceModel(m_FutureTickerPositionTableModel);
    m_FutureTickerPositionTableModel->setHeaderLabels(headerData);
    m_FutureTickerPositionTableView->setModel(m_FutureTickerPositionProxyModel);
    m_FutureTickerPositionProxyModel->setDynamicSortFilter(true);
    m_FutureTickerPositionTableView->sortByColumn(18, Qt::DescendingOrder);
    
    int column = 0;
    m_FutureTickerPositionTableView->setColumnWidth(column++, 60);
    m_FutureTickerPositionTableView->setColumnWidth(column++, 50);
    m_FutureTickerPositionTableView->setColumnWidth(column++, 80);
    m_FutureTickerPositionTableView->setColumnWidth(column++, 100);
    m_FutureTickerPositionTableView->setColumnWidth(column++, 80);
    m_FutureTickerPositionTableView->setColumnWidth(column++, 70);
    m_FutureTickerPositionTableView->setColumnWidth(column++, 100);
    m_FutureTickerPositionTableView->setColumnWidth(column++, 100);
    m_FutureTickerPositionTableView->setColumnWidth(column++, 120);
    m_FutureTickerPositionTableView->setColumnWidth(column++, 110);
    m_FutureTickerPositionTableView->setColumnWidth(column++, 130);
    m_FutureTickerPositionTableView->setColumnWidth(column++, 120);
    m_FutureTickerPositionTableView->setColumnWidth(column++, 100);
    m_FutureTickerPositionTableView->setColumnWidth(column++, 100);
    m_FutureTickerPositionTableView->setColumnWidth(column++, 120);
    m_FutureTickerPositionTableView->setColumnWidth(column++, 120);
    m_FutureTickerPositionTableView->setColumnWidth(column++, 130);
    m_FutureTickerPositionTableView->setColumnWidth(column++, 130);
    m_FutureTickerPositionTableView->setColumnWidth(column++, 120);
}

void OrderManagerWidget::InitHangingOrderTable()
{
    QStringList headerData;
    headerData << "Colo" << "Broker" << "Product" << "Account" << "Ticker" << "Exchange" << "Volume" << "Price" << "Status" 
               << "OrderSide" << "OrderType" << "EngineID" << "OrderRef" << "OrderSysID" << "OrderLocalID" << "OrderToken" 
               << "RiskID" << "ErrorID" << "ErrorMsg" << "RecvMarketTime" << "SendTime" << "InsertTime" << "BrokerACKTime" << "ExchACKTime" << "UpdateTime";
    m_HangingOrderTableView = new FinTechUI::FrozenTableView(6);
    m_HangingOrderTableView->setObjectName("HangingOrderTable");
    m_HangingOrderTableModel = new FinTechUI::XTableModel;
    m_HangingOrderTableModel->setObjectName("HangingOrderTable");
    m_HangingOrderTableModel->setHeaderLabels(headerData);
    m_HangingOrderProxyModel = new FinTechUI::XSortFilterProxyModel;
    m_HangingOrderProxyModel->setSourceModel(m_HangingOrderTableModel);
    m_HangingOrderTableView->setModel(m_HangingOrderProxyModel);
    m_HangingOrderProxyModel->setDynamicSortFilter(true);
    m_HangingOrderTableView->sortByColumn(24, Qt::DescendingOrder);
    int column = 0;
    m_HangingOrderTableView->setColumnWidth(column++, 60);
    m_HangingOrderTableView->setColumnWidth(column++, 50);
    m_HangingOrderTableView->setColumnWidth(column++, 80);
    m_HangingOrderTableView->setColumnWidth(column++, 100);
    m_HangingOrderTableView->setColumnWidth(column++, 80);
    m_HangingOrderTableView->setColumnWidth(column++, 70);
    m_HangingOrderTableView->setColumnWidth(column++, 80);
    m_HangingOrderTableView->setColumnWidth(column++, 130);
    m_HangingOrderTableView->setColumnWidth(column++, 90);
    m_HangingOrderTableView->setColumnWidth(column++, 90);
    m_HangingOrderTableView->setColumnWidth(column++, 70);
    m_HangingOrderTableView->setColumnWidth(column++, 70);
    m_HangingOrderTableView->setColumnWidth(column++, 100);
    m_HangingOrderTableView->setColumnWidth(column++, 100);
    m_HangingOrderTableView->setColumnWidth(column++, 100);
    m_HangingOrderTableView->setColumnWidth(column++, 80);
    m_HangingOrderTableView->setColumnWidth(column++, 60);
    m_HangingOrderTableView->setColumnWidth(column++, 60);
    m_HangingOrderTableView->setColumnWidth(column++, 150);
    m_HangingOrderTableView->setColumnWidth(column++, 120);
    m_HangingOrderTableView->setColumnWidth(column++, 120);
    m_HangingOrderTableView->setColumnWidth(column++, 120);
    m_HangingOrderTableView->setColumnWidth(column++, 120);
    m_HangingOrderTableView->setColumnWidth(column++, 120);
    m_HangingOrderTableView->setColumnWidth(column++, 120);
}

void OrderManagerWidget::InitHistoryOrderTable()
{
    QStringList headerData;
    headerData << "Colo" << "Broker" << "Product" << "Account" << "Ticker" << "Exchange" << "Volume" << "Price" << "Status" 
               << "OrderSide" << "OrderType" << "EngineID" << "OrderRef" << "OrderSysID" << "OrderLocalID" << "OrderToken" 
               << "RiskID" << "ErrorID" << "ErrorMsg" << "RecvMarketTime" << "SendTime" << "InsertTime" << "BrokerACKTime" << "ExchACKTime" << "UpdateTime";
    m_HistoryOrderTableView = new FinTechUI::FrozenTableView(6);
    m_HistoryOrderTableView->setObjectName("HistoryOrderTable");

    m_HistoryOrderTableModel = new FinTechUI::XTableModel;
    m_HistoryOrderTableModel->setObjectName("HistoryOrderTable");
    m_HistoryOrderTableModel->setHeaderLabels(headerData);
    m_HistoryOrderProxyModel = new FinTechUI::XSortFilterProxyModel;
    m_HistoryOrderProxyModel->setSourceModel(m_HistoryOrderTableModel);
    m_HistoryOrderTableView->setModel(m_HistoryOrderProxyModel);
    m_HistoryOrderProxyModel->setDynamicSortFilter(true);
    m_HistoryOrderTableView->sortByColumn(24, Qt::DescendingOrder);

    int column = 0;
    m_HistoryOrderTableView->setColumnWidth(column++, 60);
    m_HistoryOrderTableView->setColumnWidth(column++, 50);
    m_HistoryOrderTableView->setColumnWidth(column++, 80);
    m_HistoryOrderTableView->setColumnWidth(column++, 100);
    m_HistoryOrderTableView->setColumnWidth(column++, 80);
    m_HistoryOrderTableView->setColumnWidth(column++, 70);
    m_HistoryOrderTableView->setColumnWidth(column++, 80);
    m_HistoryOrderTableView->setColumnWidth(column++, 130);
    m_HistoryOrderTableView->setColumnWidth(column++, 90);
    m_HistoryOrderTableView->setColumnWidth(column++, 90);
    m_HistoryOrderTableView->setColumnWidth(column++, 70);
    m_HistoryOrderTableView->setColumnWidth(column++, 70);
    m_HistoryOrderTableView->setColumnWidth(column++, 100);
    m_HistoryOrderTableView->setColumnWidth(column++, 100);
    m_HistoryOrderTableView->setColumnWidth(column++, 100);
    m_HistoryOrderTableView->setColumnWidth(column++, 80);
    m_HistoryOrderTableView->setColumnWidth(column++, 60);
    m_HistoryOrderTableView->setColumnWidth(column++, 60);
    m_HistoryOrderTableView->setColumnWidth(column++, 150);
    m_HistoryOrderTableView->setColumnWidth(column++, 120);
    m_HistoryOrderTableView->setColumnWidth(column++, 120);
    m_HistoryOrderTableView->setColumnWidth(column++, 120);
    m_HistoryOrderTableView->setColumnWidth(column++, 120);
    m_HistoryOrderTableView->setColumnWidth(column++, 120);
    m_HistoryOrderTableView->setColumnWidth(column++, 120);
}

void OrderManagerWidget::InitAccountFundTable()
{
    m_AccountFundTableView = new QTableView;
    m_AccountFundTableView->setObjectName("AccountFundTable");
    m_AccountFundTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_AccountFundTableView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_AccountFundTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_AccountFundTableView->setSortingEnabled(true);
    m_AccountFundTableView->horizontalHeader()->setStretchLastSection(true);
    m_AccountFundTableView->verticalHeader()->hide();

    m_AccountFundTableModel = new FinTechUI::XTableModel;
    QStringList headerData;
    headerData << "Account" << "Available" << "Balance" << "Deposit" << "Withdraw" << "Commission" << "WithdrawQuota";
    m_AccountFundTableModel->setHeaderLabels(headerData);
    m_AccountFundProxyModel = new FinTechUI::XSortFilterProxyModel;
    m_AccountFundProxyModel->setSourceModel(m_AccountFundTableModel);
    m_AccountFundTableView->setModel(m_AccountFundProxyModel);
    m_AccountFundProxyModel->setDynamicSortFilter(true);
    m_AccountFundTableView->sortByColumn(1, Qt::AscendingOrder);
    m_AccountFundProxyModel->setColumnType(1, QVariant::Double);
    m_AccountFundProxyModel->setColumnType(2, QVariant::Double);
    m_AccountFundProxyModel->setColumnType(3, QVariant::Double);
    m_AccountFundProxyModel->setColumnType(4, QVariant::Double);
    m_AccountFundProxyModel->setColumnType(5, QVariant::Double);
    m_AccountFundProxyModel->setColumnType(6, QVariant::Double);
    int column = 0;
    m_AccountFundTableView->setColumnWidth(column++, 100);
    m_AccountFundTableView->setColumnWidth(column++, 110);
    m_AccountFundTableView->setColumnWidth(column++, 110);
    m_AccountFundTableView->setColumnWidth(column++, 90);
    m_AccountFundTableView->setColumnWidth(column++, 90);
    m_AccountFundTableView->setColumnWidth(column++, 90);
    m_AccountFundTableView->setColumnWidth(column++, 110);
    m_AccountFundTableView->setMaximumWidth(700);
}

void OrderManagerWidget::UpdateExecutionReport(const Message::PackMessage& msg)
{
    switch(msg.MessageType)
    {
    case Message::EMessageType::EAccountFund:
        UpdateAccountFund(msg);
        break;
    case Message::EMessageType::EAccountPosition:
    {
        if(Message::EBusinessType::ESTOCK == msg.AccountPosition.BussinessType)
        {
            UpdateStockAccountPosition(msg);
        }
        else if(Message::EBusinessType::ECREDIT == msg.AccountPosition.BussinessType)
        {
            UpdateStockAccountPosition(msg);
        }
        else if(Message::EBusinessType::EFUTURE == msg.AccountPosition.BussinessType)
        {
            UpdateFutureAccountPosition(msg);
        }
    }
    break;
    case Message::EMessageType::EOrderStatus:
        UpdateOrderStatus(msg);
        break;
    }
}

void OrderManagerWidget::UpdateAccountFund(const Message::PackMessage& msg)
{
    auto it = m_AccountFundMap.find(msg.AccountFund.Account);
    if(it != m_AccountFundMap.end())
    {
        UpdateAccountFund(msg.AccountFund);
    }
    else
    {
        AppendRow(msg.AccountFund);
    }
    m_AccountColoMap[msg.AccountFund.Account] = msg.AccountFund.Colo;
    m_ProductColoMap[msg.AccountFund.Product] = msg.AccountFund.Colo;
    m_AccountProductMap[msg.AccountFund.Account] = msg.AccountFund.Product;
    if(!m_AccountSet.contains(msg.AccountFund.Account))
    {
        m_AccountSet.insert(msg.AccountFund.Account);
    }
}

void OrderManagerWidget::AppendRow(const Message::TAccountFund& AccountFund)
{
    FinTechUI::XTableModelRow* ModelRow = new FinTechUI::XTableModelRow;
    FinTechUI::XTableModelItem* AccountItem = new FinTechUI::XTableModelItem(AccountFund.Account);
    ModelRow->push_back(AccountItem);
    char buffer[32] = {0};
    sprintf(buffer, "%.2f", AccountFund.Available);
    FinTechUI::XTableModelItem* AvailableItem = new FinTechUI::XTableModelItem(buffer, Qt::AlignRight | Qt::AlignVCenter);
    ModelRow->push_back(AvailableItem);
    sprintf(buffer, "%.2f", AccountFund.Balance);
    FinTechUI::XTableModelItem* BalanceItem = new FinTechUI::XTableModelItem(buffer, Qt::AlignRight | Qt::AlignVCenter);
    ModelRow->push_back(BalanceItem);
    sprintf(buffer, "%.2f", AccountFund.Deposit);
    FinTechUI::XTableModelItem* DepositItem = new FinTechUI::XTableModelItem(buffer, Qt::AlignRight | Qt::AlignVCenter);
    ModelRow->push_back(DepositItem);
    sprintf(buffer, "%.2f", AccountFund.Withdraw);
    FinTechUI::XTableModelItem* WithdrawItem = new FinTechUI::XTableModelItem(buffer, Qt::AlignRight | Qt::AlignVCenter);
    ModelRow->push_back(WithdrawItem);
    sprintf(buffer, "%.2f", AccountFund.Commission);
    FinTechUI::XTableModelItem* CommissionItem = new FinTechUI::XTableModelItem(buffer, Qt::AlignRight | Qt::AlignVCenter);
    ModelRow->push_back(CommissionItem);
    sprintf(buffer, "%.2f", AccountFund.WithdrawQuota);
    FinTechUI::XTableModelItem* WithdrawQuotaItem = new FinTechUI::XTableModelItem(buffer, Qt::AlignRight | Qt::AlignVCenter);
    ModelRow->push_back(WithdrawQuotaItem);
    FinTechUI::XTableModel::setRowBackgroundColor(ModelRow, GetAccountFundColor(AccountFund));
    m_AccountFundTableModel->appendRow(ModelRow);
    m_AccountFundMap[AccountFund.Account] = ModelRow;
}

void OrderManagerWidget::UpdateAccountFund(const Message::TAccountFund& AccountFund)
{
    FinTechUI::XTableModelRow* ModelRow = m_AccountFundMap[AccountFund.Account];
    char buffer[32] = {0};
    sprintf(buffer, "%.2f", AccountFund.Available);
    (*ModelRow)[1]->setText(buffer);
    sprintf(buffer, "%.2f", AccountFund.Balance);
    (*ModelRow)[2]->setText(buffer);
    sprintf(buffer, "%.2f", AccountFund.Deposit);
    (*ModelRow)[3]->setText(buffer);
    sprintf(buffer, "%.2f", AccountFund.Withdraw);
    (*ModelRow)[4]->setText(buffer);
    sprintf(buffer, "%.2f", AccountFund.Commission);
    (*ModelRow)[5]->setText(buffer);
    sprintf(buffer, "%.2f", AccountFund.WithdrawQuota);
    (*ModelRow)[6]->setText(buffer);

    FinTechUI::XTableModel::setRowBackgroundColor(ModelRow, GetAccountFundColor(AccountFund));
    m_AccountFundTableModel->updateRow(ModelRow);
}

void OrderManagerWidget::UpdateStockAccountPosition(const Message::PackMessage& msg)
{
    QString Key = QString(msg.AccountPosition.Account) + ":" + msg.AccountPosition.Ticker;
    auto it = m_StockTickerPositionMap.find(Key);
    if(it != m_StockTickerPositionMap.end())
    {
        UpdateStockAccountPosition(msg.AccountPosition);
    }
    else
    {
        AppendStockRow(msg.AccountPosition);
        bool ok = false;
        QStringList& Pruducts = m_ColoProductSetMap[msg.AccountPosition.Colo];
        if(!Pruducts.contains(msg.AccountPosition.Product))
        {
            Pruducts.append(msg.AccountPosition.Product);
            ok = true;
        }
        QStringList& Accounts = m_ProductAccountSetMap[msg.AccountPosition.Product];
        if(!Accounts.contains(msg.AccountPosition.Account))
        {
            Accounts.append(msg.AccountPosition.Account);
            ok = true;
        }
        QStringList& Tickers = m_AccountTickerSetMap[msg.AccountPosition.Account];
        if(!Tickers.contains(msg.AccountPosition.Ticker))
        {
            Tickers.append(msg.AccountPosition.Ticker);
            ok = true;
        }
        if(ok)
        {
            QVector<QMap<QString, QStringList>> data;
            data.append(m_ColoProductSetMap);
            data.append(m_ProductAccountSetMap);
            data.append(m_AccountTickerSetMap);
            m_FilterWidget->SetDataRelationMap(data);
        }
    }
}

void OrderManagerWidget::AppendStockRow(const Message::TAccountPosition& AccountPosition)
{
    FinTechUI::XTableModelRow* ModelRow = new FinTechUI::XTableModelRow;
    FinTechUI::XTableModelItem* ColoItem = new FinTechUI::XTableModelItem(AccountPosition.Colo);
    ModelRow->push_back(ColoItem);
    FinTechUI::XTableModelItem* BrokerItem = new FinTechUI::XTableModelItem(AccountPosition.Broker);
    ModelRow->push_back(BrokerItem);
    FinTechUI::XTableModelItem* ProductItem = new FinTechUI::XTableModelItem(AccountPosition.Product);
    ModelRow->push_back(ProductItem);
    FinTechUI::XTableModelItem* AccountItem = new FinTechUI::XTableModelItem(AccountPosition.Account);
    ModelRow->push_back(AccountItem);
    FinTechUI::XTableModelItem* TickerItem = new FinTechUI::XTableModelItem(AccountPosition.Ticker);
    ModelRow->push_back(TickerItem);
    FinTechUI::XTableModelItem* ExchangeIDItem = new FinTechUI::XTableModelItem(AccountPosition.ExchangeID);
    ModelRow->push_back(ExchangeIDItem);
    FinTechUI::XTableModelItem* LongYdPositionItem = new FinTechUI::XTableModelItem(AccountPosition.StockPosition.LongYdPosition);
    ModelRow->push_back(LongYdPositionItem);
    FinTechUI::XTableModelItem* LongPositionItem = new FinTechUI::XTableModelItem(AccountPosition.StockPosition.LongPosition);
    ModelRow->push_back(LongPositionItem);
    FinTechUI::XTableModelItem* LongTdBuyItem = new FinTechUI::XTableModelItem(AccountPosition.StockPosition.LongTdBuy);
    ModelRow->push_back(LongTdBuyItem);
    FinTechUI::XTableModelItem* LongTdSellItem = new FinTechUI::XTableModelItem(AccountPosition.StockPosition.LongTdSell);
    ModelRow->push_back(LongTdSellItem);
    FinTechUI::XTableModelItem* MarginYdPositionItem = new FinTechUI::XTableModelItem(AccountPosition.StockPosition.MarginYdPosition);
    ModelRow->push_back(MarginYdPositionItem);
    FinTechUI::XTableModelItem* MarginPositionItem = new FinTechUI::XTableModelItem(AccountPosition.StockPosition.MarginPosition);
    ModelRow->push_back(MarginPositionItem);
    FinTechUI::XTableModelItem* MarginTdBuyItem = new FinTechUI::XTableModelItem(AccountPosition.StockPosition.MarginTdBuy);
    ModelRow->push_back(MarginTdBuyItem);
    FinTechUI::XTableModelItem* MarginTdSellItem = new FinTechUI::XTableModelItem(AccountPosition.StockPosition.MarginTdSell);
    ModelRow->push_back(MarginTdSellItem);
    FinTechUI::XTableModelItem* ShortYdPositionItem = new FinTechUI::XTableModelItem(AccountPosition.StockPosition.ShortYdPosition);
    ModelRow->push_back(ShortYdPositionItem);
    FinTechUI::XTableModelItem* ShortPositionItem = new FinTechUI::XTableModelItem(AccountPosition.StockPosition.ShortPosition);
    ModelRow->push_back(ShortPositionItem);
    FinTechUI::XTableModelItem* ShortTdSellItem = new FinTechUI::XTableModelItem(AccountPosition.StockPosition.ShortTdSell);
    ModelRow->push_back(ShortTdSellItem);
    FinTechUI::XTableModelItem* ShortTdBuyItem = new FinTechUI::XTableModelItem(AccountPosition.StockPosition.ShortTdBuy);
    ModelRow->push_back(ShortTdBuyItem);
    FinTechUI::XTableModelItem* ShortDirectRepaidItem = new FinTechUI::XTableModelItem(AccountPosition.StockPosition.ShortDirectRepaid);
    ModelRow->push_back(ShortDirectRepaidItem);
    FinTechUI::XTableModelItem* SpecialPositionAvlItem = new FinTechUI::XTableModelItem(AccountPosition.StockPosition.SpecialPositionAvl);
    ModelRow->push_back(SpecialPositionAvlItem);
    FinTechUI::XTableModelItem* UpdateTimeItem = new FinTechUI::XTableModelItem(AccountPosition.UpdateTime + 11, Qt::AlignLeft | Qt::AlignVCenter);
    ModelRow->push_back(UpdateTimeItem);

    FinTechUI::XTableModel::setRowBackgroundColor(ModelRow, GetStockAccountPositionColor(AccountPosition));
    m_StockTickerPositionTableModel->appendRow(ModelRow);
    QString Key = QString(AccountPosition.Account) + ":" + AccountPosition.Ticker;
    m_StockTickerPositionMap[Key] = ModelRow;
}

void OrderManagerWidget::UpdateStockAccountPosition(const Message::TAccountPosition& AccountPosition)
{
    QString Key = QString(AccountPosition.Account) + ":" + AccountPosition.Ticker;
    FinTechUI::XTableModelRow* ModelRow = m_StockTickerPositionMap[Key];
    (*ModelRow)[6]->setText(AccountPosition.StockPosition.LongYdPosition);
    (*ModelRow)[7]->setText(AccountPosition.StockPosition.LongPosition);
    (*ModelRow)[8]->setText(AccountPosition.StockPosition.LongTdBuy);
    (*ModelRow)[9]->setText(AccountPosition.StockPosition.LongTdSell);
    (*ModelRow)[10]->setText(AccountPosition.StockPosition.MarginYdPosition);
    (*ModelRow)[11]->setText(AccountPosition.StockPosition.MarginPosition);
    (*ModelRow)[12]->setText(AccountPosition.StockPosition.MarginTdBuy);
    (*ModelRow)[13]->setText(AccountPosition.StockPosition.MarginTdSell);
    (*ModelRow)[14]->setText(AccountPosition.StockPosition.ShortYdPosition);
    (*ModelRow)[15]->setText(AccountPosition.StockPosition.ShortPosition);
    (*ModelRow)[16]->setText(AccountPosition.StockPosition.ShortTdSell);
    (*ModelRow)[17]->setText(AccountPosition.StockPosition.ShortTdBuy);
    (*ModelRow)[18]->setText(AccountPosition.StockPosition.ShortDirectRepaid);
    (*ModelRow)[19]->setText(AccountPosition.StockPosition.SpecialPositionAvl);
    (*ModelRow)[20]->setText(AccountPosition.UpdateTime + 11);
    FinTechUI::XTableModel::setRowBackgroundColor(ModelRow, GetStockAccountPositionColor(AccountPosition));
    m_StockTickerPositionTableModel->updateRow(ModelRow);
}


void OrderManagerWidget::UpdateFutureAccountPosition(const Message::PackMessage& msg)
{
    QString Key = QString(msg.AccountPosition.Account) + ":" + msg.AccountPosition.Ticker;
    auto it = m_FutureTickerPositionMap.find(Key);
    if(it != m_FutureTickerPositionMap.end())
    {
        UpdateFutureAccountPosition(msg.AccountPosition);
    }
    else
    {
        AppendFutureRow(msg.AccountPosition);

        bool ok = false;
        QStringList& Pruducts = m_ColoProductSetMap[msg.AccountPosition.Colo];
        if(!Pruducts.contains(msg.AccountPosition.Product))
        {
            Pruducts.append(msg.AccountPosition.Product);
            ok = true;
        }
        QStringList& Accounts = m_ProductAccountSetMap[msg.AccountPosition.Product];
        if(!Accounts.contains(msg.AccountPosition.Account))
        {
            Accounts.append(msg.AccountPosition.Account);
            ok = true;
        }
        QStringList& Tickers = m_AccountTickerSetMap[msg.AccountPosition.Account];
        if(!Tickers.contains(msg.AccountPosition.Ticker))
        {
            Tickers.append(msg.AccountPosition.Ticker);
            ok = true;
        }
        if(ok)
        {
            QVector<QMap<QString, QStringList>> data;
            data.append(m_ColoProductSetMap);
            data.append(m_ProductAccountSetMap);
            data.append(m_AccountTickerSetMap);
            m_FilterWidget->SetDataRelationMap(data);
        }
    }
}

void OrderManagerWidget::AppendFutureRow(const Message::TAccountPosition& AccountPosition)
{
    FinTechUI::XTableModelRow* ModelRow = new FinTechUI::XTableModelRow;
    FinTechUI::XTableModelItem* ColoItem = new FinTechUI::XTableModelItem(AccountPosition.Colo);
    ModelRow->push_back(ColoItem);
    FinTechUI::XTableModelItem* BrokerItem = new FinTechUI::XTableModelItem(AccountPosition.Broker);
    ModelRow->push_back(BrokerItem);
    FinTechUI::XTableModelItem* ProductItem = new FinTechUI::XTableModelItem(AccountPosition.Product);
    ModelRow->push_back(ProductItem);
    FinTechUI::XTableModelItem* AccountItem = new FinTechUI::XTableModelItem(AccountPosition.Account);
    ModelRow->push_back(AccountItem);
    FinTechUI::XTableModelItem* TickerItem = new FinTechUI::XTableModelItem(AccountPosition.Ticker);
    ModelRow->push_back(TickerItem);
    FinTechUI::XTableModelItem* ExchangeIDItem = new FinTechUI::XTableModelItem(AccountPosition.ExchangeID);
    ModelRow->push_back(ExchangeIDItem);
    FinTechUI::XTableModelItem* LongTdVolumeItem = new FinTechUI::XTableModelItem(AccountPosition.FuturePosition.LongTdVolume);
    ModelRow->push_back(LongTdVolumeItem);
    FinTechUI::XTableModelItem* LongYdVolumeItem = new FinTechUI::XTableModelItem(AccountPosition.FuturePosition.LongYdVolume);
    ModelRow->push_back(LongYdVolumeItem);
    FinTechUI::XTableModelItem* LongOpenVolumeItem = new FinTechUI::XTableModelItem(AccountPosition.FuturePosition.LongOpenVolume);
    ModelRow->push_back(LongOpenVolumeItem);
    FinTechUI::XTableModelItem* LongOpeningVolumeItem = new FinTechUI::XTableModelItem(AccountPosition.FuturePosition.LongOpeningVolume);
    ModelRow->push_back(LongOpeningVolumeItem);
    FinTechUI::XTableModelItem* LongClosingTdVolumeItem = new FinTechUI::XTableModelItem(AccountPosition.FuturePosition.LongClosingTdVolume);
    ModelRow->push_back(LongClosingTdVolumeItem);
    FinTechUI::XTableModelItem* LongClosingYdVolumeItem = new FinTechUI::XTableModelItem(AccountPosition.FuturePosition.LongClosingYdVolume);
    ModelRow->push_back(LongClosingYdVolumeItem);
    FinTechUI::XTableModelItem* ShortTdVolumeItem = new FinTechUI::XTableModelItem(AccountPosition.FuturePosition.ShortTdVolume);
    ModelRow->push_back(ShortTdVolumeItem);
    FinTechUI::XTableModelItem* ShortYdVolumeItem = new FinTechUI::XTableModelItem(AccountPosition.FuturePosition.ShortYdVolume);
    ModelRow->push_back(ShortYdVolumeItem);
    FinTechUI::XTableModelItem* ShortOpenVolumeItem = new FinTechUI::XTableModelItem(AccountPosition.FuturePosition.ShortOpenVolume);
    ModelRow->push_back(ShortOpenVolumeItem);
    FinTechUI::XTableModelItem* ShortOpeningVolumeItem = new FinTechUI::XTableModelItem(AccountPosition.FuturePosition.ShortOpeningVolume);
    ModelRow->push_back(ShortOpeningVolumeItem);
    FinTechUI::XTableModelItem* ShortClosingTdVolumeItem = new FinTechUI::XTableModelItem(AccountPosition.FuturePosition.ShortClosingTdVolume);
    ModelRow->push_back(ShortClosingTdVolumeItem);
    FinTechUI::XTableModelItem* ShortClosingYdVolumeItem = new FinTechUI::XTableModelItem(AccountPosition.FuturePosition.ShortClosingYdVolume);
    ModelRow->push_back(ShortClosingYdVolumeItem);
    FinTechUI::XTableModelItem* UpdateTimeItem = new FinTechUI::XTableModelItem(AccountPosition.UpdateTime + 11, Qt::AlignLeft | Qt::AlignVCenter);
    ModelRow->push_back(UpdateTimeItem);

    FinTechUI::XTableModel::setRowBackgroundColor(ModelRow, GetFutureAccountPositionColor(AccountPosition));
    m_FutureTickerPositionTableModel->appendRow(ModelRow);
    QString Key = QString(AccountPosition.Account) + ":" + AccountPosition.Ticker;
    m_FutureTickerPositionMap[Key] = ModelRow;
}

void OrderManagerWidget::UpdateFutureAccountPosition(const Message::TAccountPosition& AccountPosition)
{
    QString Key = QString(AccountPosition.Account) + ":" + AccountPosition.Ticker;
    FinTechUI::XTableModelRow* ModelRow = m_FutureTickerPositionMap[Key];
    (*ModelRow)[6]->setText(AccountPosition.FuturePosition.LongTdVolume);
    (*ModelRow)[7]->setText(AccountPosition.FuturePosition.LongYdVolume);
    (*ModelRow)[8]->setText(AccountPosition.FuturePosition.LongOpenVolume);
    (*ModelRow)[9]->setText(AccountPosition.FuturePosition.LongOpeningVolume);
    (*ModelRow)[10]->setText(AccountPosition.FuturePosition.LongClosingTdVolume);
    (*ModelRow)[11]->setText(AccountPosition.FuturePosition.LongClosingYdVolume);
    (*ModelRow)[12]->setText(AccountPosition.FuturePosition.ShortTdVolume);
    (*ModelRow)[13]->setText(AccountPosition.FuturePosition.ShortYdVolume);
    (*ModelRow)[14]->setText(AccountPosition.FuturePosition.ShortOpenVolume);
    (*ModelRow)[15]->setText(AccountPosition.FuturePosition.ShortOpeningVolume);
    (*ModelRow)[16]->setText(AccountPosition.FuturePosition.ShortClosingTdVolume);
    (*ModelRow)[17]->setText(AccountPosition.FuturePosition.ShortClosingYdVolume);
    (*ModelRow)[18]->setText(AccountPosition.UpdateTime + 11);
    FinTechUI::XTableModel::setRowBackgroundColor(ModelRow, GetFutureAccountPositionColor(AccountPosition));
    m_FutureTickerPositionTableModel->updateRow(ModelRow);
}

void OrderManagerWidget::UpdateOrderStatus(const Message::PackMessage& msg)
{
    UpdateHangingOrderTable(msg.OrderStatus);
    if(Message::EOrderStatus::EALLTRADED == msg.OrderStatus.OrderStatus || 
        Message::EOrderStatus::ECANCELLED == msg.OrderStatus.OrderStatus ||
        Message::EOrderStatus::EPARTTRADED_CANCELLED == msg.OrderStatus.OrderStatus ||
        Message::EOrderStatus::EBROKER_ERROR == msg.OrderStatus.OrderStatus ||
        Message::EOrderStatus::EEXCHANGE_ERROR == msg.OrderStatus.OrderStatus ||
        Message::EOrderStatus::ERISK_ORDER_REJECTED == msg.OrderStatus.OrderStatus ||
        Message::EOrderStatus::ERISK_CHECK_INIT == msg.OrderStatus.OrderStatus ||
        Message::EOrderStatus::ERISK_CHECK_SELFMATCH == msg.OrderStatus.OrderStatus)
    {
        UpdateHistoryOrderTable(msg.OrderStatus);
    }

    bool ok = false;
    QStringList& Pruducts = m_ColoProductSetMap[msg.OrderStatus.Colo];
    if(!Pruducts.contains(msg.OrderStatus.Product))
    {
        Pruducts.append(msg.OrderStatus.Product);
        ok = true;
    }
    QStringList& Accounts = m_ProductAccountSetMap[msg.OrderStatus.Product];
    if(!Accounts.contains(msg.OrderStatus.Account))
    {
        Accounts.append(msg.OrderStatus.Account);
        ok = true;
    }
    QStringList& Tickers = m_AccountTickerSetMap[msg.OrderStatus.Account];
    if(!Tickers.contains(msg.OrderStatus.Ticker))
    {
        Tickers.append(msg.OrderStatus.Ticker);
        ok = true;
    }
    if(ok)
    {
        QVector<QMap<QString, QStringList>> data;
        data.append(m_ColoProductSetMap);
        data.append(m_ProductAccountSetMap);
        data.append(m_AccountTickerSetMap);
        m_FilterWidget->SetDataRelationMap(data);
    }
}

void OrderManagerWidget::UpdateHangingOrderTable(const Message::TOrderStatus& OrderStatus)
{
    if(Message::EOrderStatus::EORDER_SENDED == OrderStatus.OrderStatus)
    {
        AppendRow(OrderStatus, m_HangingOrderTableModel);
    }
    else if(Message::EOrderStatus::EALLTRADED == OrderStatus.OrderStatus || 
        Message::EOrderStatus::ECANCELLED == OrderStatus.OrderStatus ||
        Message::EOrderStatus::EPARTTRADED_CANCELLED == OrderStatus.OrderStatus ||
        Message::EOrderStatus::EBROKER_ERROR == OrderStatus.OrderStatus ||
        Message::EOrderStatus::EEXCHANGE_ERROR == OrderStatus.OrderStatus ||
        Message::EOrderStatus::ERISK_ORDER_REJECTED == OrderStatus.OrderStatus)
    {
        RemoveRow(OrderStatus, m_HangingOrderTableModel);
    }
    else
    {
        UpdateRow(OrderStatus, m_HangingOrderTableModel);
    }
}

void OrderManagerWidget::UpdateHistoryOrderTable(const Message::TOrderStatus& OrderStatus)
{
    AppendRow(OrderStatus, m_HistoryOrderTableModel);
}

void OrderManagerWidget::AppendRow(const Message::TOrderStatus& OrderStatus, FinTechUI::XTableModel* tableModel)
{
    if(tableModel == NULL)
    {
        return;
    }
    FinTechUI::XTableModelRow* ModelRow = new FinTechUI::XTableModelRow;
    FinTechUI::XTableModelItem* ColoItem = new FinTechUI::XTableModelItem(OrderStatus.Colo);
    ModelRow->push_back(ColoItem);
    FinTechUI::XTableModelItem* BrokerItem = new FinTechUI::XTableModelItem(OrderStatus.Broker);
    ModelRow->push_back(BrokerItem);
    FinTechUI::XTableModelItem* ProductItem = new FinTechUI::XTableModelItem(OrderStatus.Product);
    ModelRow->push_back(ProductItem);
    FinTechUI::XTableModelItem* AccountItem = new FinTechUI::XTableModelItem(OrderStatus.Account);
    ModelRow->push_back(AccountItem);
    FinTechUI::XTableModelItem* TickerItem = new FinTechUI::XTableModelItem(OrderStatus.Ticker);
    ModelRow->push_back(TickerItem);
    FinTechUI::XTableModelItem* ExchangeIDItem = new FinTechUI::XTableModelItem(OrderStatus.ExchangeID);
    ModelRow->push_back(ExchangeIDItem);
    char buffer[32] = {0};
    sprintf(buffer, "%d/%d", OrderStatus.TotalTradedVolume, OrderStatus.SendVolume);
    FinTechUI::XTableModelItem* VolumeItem = new FinTechUI::XTableModelItem(buffer);
    ModelRow->push_back(VolumeItem);
    sprintf(buffer, "%.3f/%.3f", OrderStatus.TradedAvgPrice, OrderStatus.SendPrice);
    FinTechUI::XTableModelItem* PriceItem = new FinTechUI::XTableModelItem(buffer);
    ModelRow->push_back(PriceItem);
    FinTechUI::XTableModelItem* StatusItem = new FinTechUI::XTableModelItem(GetOrderStatus(OrderStatus.OrderStatus));
    ModelRow->push_back(StatusItem);
    QString OrderSide;
    if(Message::EBusinessType::EFUTURE == OrderStatus.BussinessType)
    {
        OrderSide = GetFutureOrderSide(OrderStatus.OrderSide);
    }
    else if(Message::EBusinessType::ESTOCK == OrderStatus.BussinessType)
    {
        OrderSide = GetStockOrderSide(OrderStatus.OrderSide);
    }
    else if(Message::EBusinessType::ECREDIT == OrderStatus.BussinessType)
    {
        OrderSide = GetStockOrderSide(OrderStatus.OrderSide);
    }
    FinTechUI::XTableModelItem* OrderSideItem = new FinTechUI::XTableModelItem(OrderSide);
    ModelRow->push_back(OrderSideItem);
    FinTechUI::XTableModelItem* OrderTypeItem = new FinTechUI::XTableModelItem(GetOrderType(OrderStatus.OrderType));
    ModelRow->push_back(OrderTypeItem);
    sprintf(buffer, "0X%X", OrderStatus.EngineID);
    FinTechUI::XTableModelItem* EngineIDItem = new FinTechUI::XTableModelItem(buffer);
    ModelRow->push_back(EngineIDItem);
    FinTechUI::XTableModelItem* OrderRefItem = new FinTechUI::XTableModelItem(OrderStatus.OrderRef);
    ModelRow->push_back(OrderRefItem);
    FinTechUI::XTableModelItem* OrderSysIDItem = new FinTechUI::XTableModelItem(OrderStatus.OrderSysID);
    ModelRow->push_back(OrderSysIDItem);
    FinTechUI::XTableModelItem* OrderLocalIDItem = new FinTechUI::XTableModelItem(OrderStatus.OrderLocalID);
    ModelRow->push_back(OrderLocalIDItem);
    FinTechUI::XTableModelItem* OrderTokenItem = new FinTechUI::XTableModelItem(OrderStatus.OrderToken);
    ModelRow->push_back(OrderTokenItem);
    FinTechUI::XTableModelItem* RiskIDItem = new FinTechUI::XTableModelItem(OrderStatus.RiskID);
    ModelRow->push_back(RiskIDItem);
    FinTechUI::XTableModelItem* ErrorIDItem = new FinTechUI::XTableModelItem(OrderStatus.ErrorID);
    ModelRow->push_back(ErrorIDItem);
    FinTechUI::XTableModelItem* ErrorMsgItem = new FinTechUI::XTableModelItem(OrderStatus.ErrorMsg, Qt::AlignLeft | Qt::AlignVCenter);
    ModelRow->push_back(ErrorMsgItem);
    FinTechUI::XTableModelItem* RecvMarketTimeItem = new FinTechUI::XTableModelItem(OrderStatus.RecvMarketTime + 11);
    ModelRow->push_back(RecvMarketTimeItem);
    FinTechUI::XTableModelItem* SendTimeItem = new FinTechUI::XTableModelItem(OrderStatus.SendTime + 11);
    ModelRow->push_back(SendTimeItem);
    FinTechUI::XTableModelItem* InsertTimeItem = new FinTechUI::XTableModelItem(OrderStatus.InsertTime + 11);
    ModelRow->push_back(InsertTimeItem);
    FinTechUI::XTableModelItem* BrokerACKTimeItem = new FinTechUI::XTableModelItem(OrderStatus.BrokerACKTime + 11);
    ModelRow->push_back(BrokerACKTimeItem);
    FinTechUI::XTableModelItem* ExchangeACKTimeItem = new FinTechUI::XTableModelItem(OrderStatus.ExchangeACKTime + 11);
    ModelRow->push_back(ExchangeACKTimeItem);
    FinTechUI::XTableModelItem* UpdateTimeItem = new FinTechUI::XTableModelItem(OrderStatus.UpdateTime + 11, Qt::AlignLeft | Qt::AlignVCenter);
    ModelRow->push_back(UpdateTimeItem);

    FinTechUI::XTableModel::setRowBackgroundColor(ModelRow, GetOrderStatusColor(OrderStatus));

    tableModel->appendRow(ModelRow);
    if(Message::EOrderStatus::EORDER_SENDED == OrderStatus.OrderStatus)
    {
        QString Key = QString(OrderStatus.Account) + ":" + OrderStatus.OrderRef;
        auto it = m_HangingOrderMap.find(Key);
        if(it == m_HangingOrderMap.end())
        {
            m_HangingOrderMap[Key] = ModelRow;
        }
    }
}

void OrderManagerWidget::UpdateRow(const Message::TOrderStatus& OrderStatus, FinTechUI::XTableModel* tableModel)
{
    QString Key = QString(OrderStatus.Account) + ":" + OrderStatus.OrderRef;
    auto it = m_HangingOrderMap.find(Key);
    if(it != m_HangingOrderMap.end())
    {
        FinTechUI::XTableModelRow* ModelRow = m_HangingOrderMap[Key];
        char buffer[32] = {0};
        sprintf(buffer, "%d/%d", OrderStatus.TotalTradedVolume, OrderStatus.SendVolume);
        (*ModelRow)[6]->setText(buffer);
        sprintf(buffer, "%.3f/%.3f", OrderStatus.TradedAvgPrice, OrderStatus.SendPrice);
        (*ModelRow)[7]->setText(buffer);
        (*ModelRow)[8]->setText(GetOrderStatus(OrderStatus.OrderStatus));

        (*ModelRow)[12]->setText(OrderStatus.OrderRef);
        (*ModelRow)[13]->setText(OrderStatus.OrderSysID);
        (*ModelRow)[14]->setText(OrderStatus.OrderLocalID);
        (*ModelRow)[16]->setText(OrderStatus.RiskID);
        (*ModelRow)[17]->setText(OrderStatus.ErrorID);
        (*ModelRow)[18]->setText(OrderStatus.ErrorMsg);
        (*ModelRow)[19]->setText(OrderStatus.RecvMarketTime + 11);
        (*ModelRow)[20]->setText(OrderStatus.SendTime + 11);
        (*ModelRow)[21]->setText(OrderStatus.InsertTime + 11);
        (*ModelRow)[22]->setText(OrderStatus.BrokerACKTime + 11);
        (*ModelRow)[23]->setText(OrderStatus.ExchangeACKTime + 11);
        (*ModelRow)[24]->setText(OrderStatus.UpdateTime + 11);

        FinTechUI::XTableModel::setRowBackgroundColor(ModelRow, GetOrderStatusColor(OrderStatus));
        tableModel->updateRow(ModelRow);
    }
}

void OrderManagerWidget::RemoveRow(const Message::TOrderStatus& OrderStatus, FinTechUI::XTableModel* tableModel)
{
    QString Key = QString(OrderStatus.Account) + ":" + OrderStatus.OrderRef;
    auto it = m_HangingOrderMap.find(Key);
    if(it != m_HangingOrderMap.end())
    {
        tableModel->deleteRow(m_HangingOrderMap[Key], true);
        m_HangingOrderMap.erase(it);
    }
}

void OrderManagerWidget::CancelOrderDialog(const QString& account, const QString& OrderID)
{
    QDialog dialog;
    dialog.setWindowTitle("Cancel Order");
    dialog.setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint | Qt::WindowStaysOnTopHint);
    dialog.setFixedSize(400, 300);

    QFormLayout* formLayout = new QFormLayout;
    formLayout->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);
    QComboBox* accountCombo = new QComboBox;
    QStringList accounts = m_AccountSet.toList();
    accounts.sort();
    accountCombo->addItems(accounts);
    accountCombo->setCurrentText(account);
    formLayout->addRow("Account: ", accountCombo);
    
    QLineEdit* OrderRefEdit = new QLineEdit;
    formLayout->addRow("OrderRef: ", OrderRefEdit);
    OrderRefEdit->setText(OrderID);

    QComboBox* riskCheckCombo = new QComboBox;
    riskCheckCombo->addItems(QStringList() << "Check" << "NoCheck");
    formLayout->addRow("RiskCheck: ", riskCheckCombo);
    riskCheckCombo->setCurrentText("Check");

    QComboBox* engineCombo = new QComboBox;
    engineCombo->addItems(QStringList() << "None" << "TraderOrder" << "ForceEndOrder");
    formLayout->addRow("Engine: ", engineCombo);
    engineCombo->setCurrentText("None");

    QDialogButtonBox* buttonBox = new QDialogButtonBox;
    buttonBox->setCenterButtons(true);
    buttonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    formLayout->addWidget(buttonBox);
    connect(buttonBox, SIGNAL(accepted()), &dialog, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), &dialog, SLOT(reject()));

    dialog.setLayout(formLayout);
    if(dialog.exec() == QDialog::Accepted)
    {
        Message::PackMessage message;
        memset(&message, 0, sizeof (message));
        message.MessageType = Message::EMessageType::EActionRequest;
        QString Account = accountCombo->currentText().trimmed();
        QString OrderRef = OrderRefEdit->text().trimmed();
        if(riskCheckCombo->currentText() == "Check")
        {
            message.ActionRequest.RiskStatus = Message::ERiskStatusType::EPREPARE_CHECKED;
        }
        else if(riskCheckCombo->currentText() == "NoCheck")
        {
            message.ActionRequest.RiskStatus = Message::ERiskStatusType::ENOCHECKED;
        }
        if(engineCombo->currentText() == "TraderOrder")
        {
            message.ActionRequest.EngineID = Message::EEngineType::ETRADER_ORDER;
        }
        else if(engineCombo->currentText() == "ForceEndOrder")
        {
            QString Key = account + ":" + OrderID;
            auto it = m_HangingOrderMap.find(Key);
            if(it != m_HangingOrderMap.end())
            {
                FinTechUI::XTableModelRow* ModelRow = m_HangingOrderMap[Key];
                FinTechUI::XTableModelRow* NewModelRow = new FinTechUI::XTableModelRow;
                for(int i = 0; i < ModelRow->size(); i++)
                {
                    FinTechUI::XTableModelItem* Item = new FinTechUI::XTableModelItem;
                    memcpy(Item, ModelRow->at(i), sizeof(*Item));
                    NewModelRow->push_back(Item);
                }
                (*NewModelRow)[17]->setText(-1);
                (*NewModelRow)[18]->setText("Force End Order");
                (*NewModelRow)[24]->setText(Utils::getCurrentTimeUs() + 11);
                m_HistoryOrderTableModel->appendRow(NewModelRow);
                m_HangingOrderTableModel->deleteRow(ModelRow, true);
                m_HangingOrderMap.erase(it);
            }
            return;
        }
        memcpy(message.ActionRequest.Colo, m_AccountColoMap[Account].toStdString().c_str(), sizeof(message.ActionRequest.Colo));
        memcpy(message.ActionRequest.Account, Account.toStdString().c_str(), sizeof(message.ActionRequest.Account));
        memcpy(message.ActionRequest.OrderRef, OrderRef.toStdString().c_str(), sizeof(message.ActionRequest.OrderRef));
        HPPackClient::SendData(reinterpret_cast<unsigned char *>(&message), sizeof(message));
        Utils::gLogger->Log->info("OrderManagerWidget::CancelOrderDialog Colo:{} Account:{} OrderRef:{} RiskStatus:{}", 
                                  message.ActionRequest.Colo, message.ActionRequest.Account, message.ActionRequest.OrderRef, message.ActionRequest.RiskStatus);
    }
}

void OrderManagerWidget::ExportTable(FinTechUI::XTableModel* tableModel, const QString& path)
{
    if(NULL != tableModel)
    {
        QStringList headers = tableModel->headerLabels();
        QString fileName = tableModel->objectName();
        QString filePath = path + "/" + fileName + QDateTime::currentDateTime().toString("yyyyMMdd") + ".csv";
        QFile file(filePath);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
        {
            file.write((headers.join(",") + "\n").toStdString().c_str());
            for(int j = 0; j < tableModel->rowCount(); j++)
            {
                QStringList LineItems;
                for(int k = 0; k < tableModel->columnCount(); k++)
                {
                    LineItems << QString("\'") + tableModel->itemText(j, k).trimmed() + QString("\'");
                }
                file.write((LineItems.join(",") + "\n").toStdString().c_str());
            }
        }
        file.close();
        Utils::gLogger->Log->info("OrderManagerWidget::ExportTable {}", path.toStdString());
        QMessageBox::information(this, "Export Order", QString("Export Order to %1").arg(filePath));
    }
}

void OrderManagerWidget::timerEvent(QTimerEvent *event)
{
    // flush every 200ms
    if(event->timerId() == m_FlushTimer)
    {
        while(!m_ExecutionReportQueue.empty())
        {
            Message::PackMessage data;
            memcpy(&data, &m_ExecutionReportQueue.head(), sizeof(data));
            UpdateExecutionReport(data);
            m_ExecutionReportQueue.dequeue();
        }
    }
}
