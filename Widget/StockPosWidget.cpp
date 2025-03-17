#include "StockPosWidget.h"

StockPosWidget::StockPosWidget(const std::unordered_map<int, std::string>& strategy_map, QWidget *parent) : FinTechUI::TabPageWidget(parent)
{
    m_StrategyPropertyMap = strategy_map;

    m_Splitter = new QSplitter(Qt::Horizontal);

    m_LeftWidget = new QWidget;
    m_FilterWidget = new FinTechUI::FilterWidget;
    QStringList filter;
    filter << "Strategy" << "Account" << "Ticker";
    m_FilterWidget->SetHeaderLabels(filter);
    m_FilterWidget->SetColumnWidth("Strategy", 120);
    m_FilterWidget->SetColumnWidth("Account", 90);
    m_FilterWidget->SetColumnWidth("Ticker", 90);
    m_LeftWidget->setMinimumWidth(200);
    m_LeftWidget->setMaximumWidth(m_FilterWidget->width());

    m_RightWidget = new QWidget;
    m_PosTableView = new QTableView;
    m_PosTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_PosTableView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_PosTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_PosTableView->setSortingEnabled(true);
    m_PosTableView->horizontalHeader()->setStretchLastSection(true);
    m_PosTableView->verticalHeader()->hide();
    
    m_PosModel = new FinTechUI::XTableModel;
    QStringList headerData;
    headerData << "Strategy" << "Account" << "Ticker" << "Product" << "Colo" 
                << "Buy"  << "OpenLong" << "ShortTdBuy" << "MarginTdBuy" 
                << "Sell" << "CloseLong" << "ShortTdSell" << "MarginTdSell" << "UpdateTime";
    m_PosModel->setHeaderLabels(headerData);
    m_XSortFilterProxyModel = new FinTechUI::XSortFilterProxyModel;
    m_XSortFilterProxyModel->setSourceModel(m_PosModel);
    m_PosTableView->setModel(m_XSortFilterProxyModel);
    m_XSortFilterProxyModel->setDynamicSortFilter(true);
    m_PosTableView->sortByColumn(0, Qt::DescendingOrder);
    int column = 0;
    m_PosTableView->setColumnWidth(column++, 120);
    m_PosTableView->setColumnWidth(column++, 120);
    m_PosTableView->setColumnWidth(column++, 90);
    m_PosTableView->setColumnWidth(column++, 100);
    m_PosTableView->setColumnWidth(column++, 80);
    m_PosTableView->setColumnWidth(column++, 100);
    m_PosTableView->setColumnWidth(column++, 100);
    m_PosTableView->setColumnWidth(column++, 100);
    m_PosTableView->setColumnWidth(column++, 100);
    m_PosTableView->setColumnWidth(column++, 100);
    m_PosTableView->setColumnWidth(column++, 100);
    m_PosTableView->setColumnWidth(column++, 100);
    m_PosTableView->setColumnWidth(column++, 100);
    m_PosTableView->setColumnWidth(column++, 120);

    QVBoxLayout* leftVLayout = new QVBoxLayout;
    leftVLayout->setContentsMargins(0, 0, 0, 0);
    leftVLayout->addWidget(m_FilterWidget);
    m_LeftWidget->setLayout(leftVLayout);
    QVBoxLayout* rightVLayout = new QVBoxLayout;
    rightVLayout->setContentsMargins(0, 0, 0, 0);
    rightVLayout->addWidget(m_PosTableView);
    m_RightWidget->setLayout(rightVLayout);

    m_Splitter->addWidget(m_LeftWidget);
    m_Splitter->addWidget(m_RightWidget);
    m_Splitter->setStretchFactor(0, 2);
    m_Splitter->setStretchFactor(1, 8);
    m_Splitter->setCollapsible(0, true);
    m_Splitter->setCollapsible(1, false);

    m_HBoxLayout = new QHBoxLayout;
    m_HBoxLayout->setContentsMargins(0, 0, 0, 0);
    m_HBoxLayout->addWidget(m_Splitter);
    setLayout(m_HBoxLayout);

    connect(m_FilterWidget, &FinTechUI::FilterWidget::FilterChanged, this, &StockPosWidget::OnFilterTable, Qt::UniqueConnection);
}

void StockPosWidget::OnReceivedOrderStatus(const QList<Message::PackMessage>& items)
{
    for(int i = 0; i < items.size(); i++)
    {
        FMTLOG(fmtlog::INF, "StockPosWidget::OnReceivedOrderStatus Account:{} Ticker:{}", items.at(i).OrderStatus.Account, items.at(i).OrderStatus.Ticker);
        UpdatePosition(items.at(i));
    }
}
 
void StockPosWidget::OnFilterTable(const QVector<QStringList>& filter)
{
    QStringList StrategyFilter = filter.at(0);
    QStringList AccountFilter = filter.at(1);
    QStringList TickerFilter = filter.at(2);
    m_Filter[0] = StrategyFilter;
    m_Filter[1] = AccountFilter;
    m_Filter[2] = TickerFilter;
    m_XSortFilterProxyModel->setRowFilter(m_Filter);
    m_XSortFilterProxyModel->resetFilter();
}

void StockPosWidget::UpdatePosition(const Message::PackMessage& msg)
{
    if(Message::EOrderStatusType::EPARTTRADED == msg.OrderStatus.OrderStatus 
        || Message::EOrderStatusType::EALLTRADED == msg.OrderStatus.OrderStatus 
        || Message::EOrderStatusType::EPARTTRADED_CANCELLED == msg.OrderStatus.OrderStatus)
    {
        auto it = m_StrategyPropertyMap.find(msg.OrderStatus.EngineID);
        if(it != m_StrategyPropertyMap.end())
        {
            QString StrategyName = it->second.c_str();
            QString Account = msg.OrderStatus.Account;
            QString Ticker = msg.OrderStatus.Ticker;
            QString Key = StrategyName + ":" + Account + ":" + Ticker;
            if(m_StrategyAccountTickerModelRowMap.contains(Key))
            {
                UpdateRow(StrategyName, msg);
            }
            else
            {
                AppendRow(StrategyName, msg);
            }
            // 汇总一个账户下不同策略的持仓
            StrategyName = "*";
            Key = StrategyName + ":" + Account + ":" + Ticker;
            if(m_StrategyAccountTickerModelRowMap.contains(Key))
            {
                UpdateRow(StrategyName, msg);
            }
            else
            {
                AppendRow(StrategyName, msg);
            }
        }
    }
}

void StockPosWidget::AppendRow(const QString& Strategy, const Message::PackMessage& msg)
{
    QString Account = msg.OrderStatus.Account;
    QString Ticker = msg.OrderStatus.Ticker;
    QString Key = Strategy + ":" + Account + ":" + Ticker;

    FinTechUI::XTableModelRow* ModelRow = new FinTechUI::XTableModelRow;
    FinTechUI::XTableModelItem* StrategyItem = new FinTechUI::XTableModelItem(Strategy);
    ModelRow->push_back(StrategyItem);
    FinTechUI::XTableModelItem* AccountItem = new FinTechUI::XTableModelItem(msg.OrderStatus.Account);
    ModelRow->push_back(AccountItem);
    FinTechUI::XTableModelItem* TickerItem = new FinTechUI::XTableModelItem(msg.OrderStatus.Ticker);
    ModelRow->push_back(TickerItem);
    FinTechUI::XTableModelItem* ProductItem = new FinTechUI::XTableModelItem(msg.OrderStatus.Product);
    ModelRow->push_back(ProductItem);
    FinTechUI::XTableModelItem* ColoItem = new FinTechUI::XTableModelItem(msg.OrderStatus.Colo);
    ModelRow->push_back(ColoItem);
    StockPosition& position = m_StrategyPositionMap[Key];
    if(Message::EOrderSide::EOPEN_LONG == msg.OrderStatus.OrderSide)
    {
        position.OpenLong += msg.OrderStatus.TradedVolume;
    }
    else if(Message::EOrderSide::ESIDE_COLLATERAL_BUY == msg.OrderStatus.OrderSide)
    {
        position.OpenLong += msg.OrderStatus.TradedVolume;
    }
    else if(Message::EOrderSide::ESIDE_MARGIN_BUY == msg.OrderStatus.OrderSide)
    {
        position.MarginTdBuy += msg.OrderStatus.TradedVolume;
    }
    else if(Message::EOrderSide::ESIDE_REPAY_STOCK_BY_BUY == msg.OrderStatus.OrderSide)
    {
        position.ShortTdBuy += msg.OrderStatus.TradedVolume;
    }
    else if(Message::EOrderSide::ECLOSE_LONG == msg.OrderStatus.OrderSide)
    {
        position.CloseLong += msg.OrderStatus.TradedVolume;
    }
    else if(Message::EOrderSide::ESIDE_COLLATERAL_SELL == msg.OrderStatus.OrderSide)
    {
        position.CloseLong += msg.OrderStatus.TradedVolume;
    }
    else if(Message::EOrderSide::ESIDE_REPAY_MARGIN_BY_SELL == msg.OrderStatus.OrderSide)
    {
        position.MarginTdSell += msg.OrderStatus.TradedVolume;
    }
    else if(Message::EOrderSide::ESIDE_SHORT_SELL == msg.OrderStatus.OrderSide)
    {
        position.ShortTdSell += msg.OrderStatus.TradedVolume;
    }
    int Buy = position.OpenLong + position.ShortTdBuy + position.MarginTdBuy;
    FinTechUI::XTableModelItem* BuyItem = new FinTechUI::XTableModelItem(Buy);
    ModelRow->push_back(BuyItem);
    FinTechUI::XTableModelItem* OpenLongItem = new FinTechUI::XTableModelItem(position.OpenLong);
    ModelRow->push_back(OpenLongItem);
    FinTechUI::XTableModelItem* ShortTdBuyItem = new FinTechUI::XTableModelItem(position.ShortTdBuy);
    ModelRow->push_back(ShortTdBuyItem);
    FinTechUI::XTableModelItem* MarginTdBuyItem = new FinTechUI::XTableModelItem(position.MarginTdBuy);
    ModelRow->push_back(MarginTdBuyItem);

    int Sell = position.CloseLong + position.ShortTdSell + position.MarginTdSell;
    FinTechUI::XTableModelItem* SellItem = new FinTechUI::XTableModelItem(Sell);
    ModelRow->push_back(SellItem);
    FinTechUI::XTableModelItem* CloseLongItem = new FinTechUI::XTableModelItem(position.CloseLong);
    ModelRow->push_back(CloseLongItem);
    FinTechUI::XTableModelItem* ShortTdSellItem = new FinTechUI::XTableModelItem(position.ShortTdSell);
    ModelRow->push_back(ShortTdSellItem);
    FinTechUI::XTableModelItem* MarginTdSellItem = new FinTechUI::XTableModelItem(position.MarginTdSell);
    ModelRow->push_back(MarginTdSellItem);

    FinTechUI::XTableModelItem* UpdateTimetem = new FinTechUI::XTableModelItem(msg.OrderStatus.UpdateTime, Qt::AlignCenter);
    ModelRow->push_back(UpdateTimetem);

    if(position.OpenLong > 0 || position.ShortTdSell > 0)
    {
        FinTechUI::XTableModel::setRowBackgroundColor(ModelRow, QColor("#FFA500"));
    }
    else
    {
        FinTechUI::XTableModel::setRowBackgroundColor(ModelRow, QColor("#00CED1"));
    }
    m_PosModel->appendRow(ModelRow);
    m_StrategyAccountTickerModelRowMap[Key] = ModelRow;

    bool ok = false;
    QStringList& Accounts = m_StrategyAccountSetMap[Strategy];
    if(!Accounts.contains(Account))
    {
        Accounts.append(Account);
        ok = true;
    }
    QStringList& Tickers = m_AccountTickerSetMap[Account];
    if(!Tickers.contains(Ticker))
    {
        Tickers.append(Ticker);
        ok = true;
    }
    if(ok)
    {
        QVector<QMap<QString, QStringList>> data;
        data.append(m_StrategyAccountSetMap);
        data.append(m_AccountTickerSetMap);
        m_FilterWidget->SetDataRelationMap(data);
    }
}

void StockPosWidget::UpdateRow(const QString& Strategy, const Message::PackMessage& msg)
{
    QString Account = msg.OrderStatus.Account;
    QString Ticker = msg.OrderStatus.Ticker;
    QString Key = Strategy + ":" + Account + ":" + Ticker;
    StockPosition& position = m_StrategyPositionMap[Key];
    if(Message::EOrderSide::EOPEN_LONG == msg.OrderStatus.OrderSide)
    {
        position.OpenLong += msg.OrderStatus.TradedVolume;
    }
    else if(Message::EOrderSide::ESIDE_COLLATERAL_BUY == msg.OrderStatus.OrderSide)
    {
        position.OpenLong += msg.OrderStatus.TradedVolume;
    }
    else if(Message::EOrderSide::ESIDE_MARGIN_BUY == msg.OrderStatus.OrderSide)
    {
        position.MarginTdBuy += msg.OrderStatus.TradedVolume;
    }
    else if(Message::EOrderSide::ESIDE_REPAY_STOCK_BY_BUY == msg.OrderStatus.OrderSide)
    {
        position.ShortTdBuy += msg.OrderStatus.TradedVolume;
    }
    else if(Message::EOrderSide::ECLOSE_LONG == msg.OrderStatus.OrderSide)
    {
        position.CloseLong += msg.OrderStatus.TradedVolume;
    }
    else if(Message::EOrderSide::ESIDE_COLLATERAL_SELL == msg.OrderStatus.OrderSide)
    {
        position.CloseLong += msg.OrderStatus.TradedVolume;
    }
    else if(Message::EOrderSide::ESIDE_REPAY_MARGIN_BY_SELL == msg.OrderStatus.OrderSide)
    {
        position.MarginTdSell += msg.OrderStatus.TradedVolume;
    }
    else if(Message::EOrderSide::ESIDE_SHORT_SELL == msg.OrderStatus.OrderSide)
    {
        position.ShortTdSell += msg.OrderStatus.TradedVolume;
    }
    int Buy = position.OpenLong + position.ShortTdBuy + position.MarginTdBuy;
    int Sell = position.CloseLong + position.ShortTdSell + position.MarginTdSell;
    FinTechUI::XTableModelRow* ModelRow = m_StrategyAccountTickerModelRowMap[Key];
    (*ModelRow)[5]->setText(Buy);
    (*ModelRow)[6]->setText(position.OpenLong);
    (*ModelRow)[7]->setText(position.ShortTdBuy);
    (*ModelRow)[8]->setText(position.MarginTdBuy);

    (*ModelRow)[9]->setText(Sell);
    (*ModelRow)[10]->setText(position.CloseLong);
    (*ModelRow)[11]->setText(position.ShortTdSell);
    (*ModelRow)[12]->setText(position.MarginTdSell);

    (*ModelRow)[13]->setText(msg.OrderStatus.UpdateTime);

    if(position.OpenLong > 0 || position.ShortTdSell > 0)
    {
        FinTechUI::XTableModel::setRowBackgroundColor(ModelRow, QColor("#FFA500"));
    }
    else
    {
        FinTechUI::XTableModel::setRowBackgroundColor(ModelRow, QColor("#00CED1"));
    }
    m_PosModel->updateRow(ModelRow);
}
