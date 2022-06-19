#include "EventLogWidget.h"
extern Utils::Logger *gLogger;

EventLogWidget::EventLogWidget(QWidget *parent) : FinTechUI::TabPageWidget(parent)
{
    m_Splitter = new QSplitter(Qt::Horizontal);

    m_LeftWidget = new QWidget;
    m_FilterWidget = new FinTechUI::FilterWidget;
    QStringList filter;
    filter << "Colo" <<  "App" << "Account" << "Ticker";
    m_FilterWidget->SetHeaderLabels(filter);
    m_FilterWidget->SetColumnWidth("Colo", 70);
    m_FilterWidget->SetColumnWidth("App", 80);
    m_FilterWidget->SetColumnWidth("Account", 90);
    m_FilterWidget->SetColumnWidth("Ticker", 90);
    m_LeftWidget->setMinimumWidth(200);
    m_LeftWidget->setMaximumWidth(m_FilterWidget->width());

    m_RightWidget = new QWidget;
    m_EventLogTableView = new QTableView;
    m_EventLogTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_EventLogTableView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_EventLogTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_EventLogTableView->setSortingEnabled(true);
    m_EventLogTableView->horizontalHeader()->setStretchLastSection(true);
    m_EventLogTableView->verticalHeader()->hide();
    
    m_EventLogModel = new FinTechUI::XTableModel;
    QStringList headerData;
    headerData << "Colo" << "App" << "Broker" << "Product" << "Account" << "Ticker"  << "UpdateTime" << "Event";
    m_EventLogModel->setHeaderLabels(headerData);
    m_XSortFilterProxyModel = new FinTechUI::XSortFilterProxyModel;
    m_XSortFilterProxyModel->setSourceModel(m_EventLogModel);
    m_EventLogTableView->setModel(m_XSortFilterProxyModel);
    m_XSortFilterProxyModel->setDynamicSortFilter(true);
    m_EventLogTableView->sortByColumn(6, Qt::DescendingOrder);
    int column = 0;
    m_EventLogTableView->setColumnWidth(column++, 80);
    m_EventLogTableView->setColumnWidth(column++, 80);
    m_EventLogTableView->setColumnWidth(column++, 70);
    m_EventLogTableView->setColumnWidth(column++, 80);
    m_EventLogTableView->setColumnWidth(column++, 90);
    m_EventLogTableView->setColumnWidth(column++, 80);
    m_EventLogTableView->setColumnWidth(column++, 120);
    m_EventLogTableView->setColumnWidth(column++, 500);

    QVBoxLayout* leftVLayout = new QVBoxLayout;
    leftVLayout->setContentsMargins(0, 0, 0, 0);
    leftVLayout->addWidget(m_FilterWidget);
    m_LeftWidget->setLayout(leftVLayout);
    QVBoxLayout* rightVLayout = new QVBoxLayout;
    rightVLayout->setContentsMargins(0, 0, 0, 0);
    rightVLayout->addWidget(m_EventLogTableView);
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

    connect(m_FilterWidget, &FinTechUI::FilterWidget::FilterChanged, this, &EventLogWidget::OnFilterTable, Qt::UniqueConnection);
}

void EventLogWidget::OnReceivedEventLog(const QList<Message::PackMessage>& items)
{
    for(int i = 0; i < items.size(); i++)
    {
        Utils::gLogger->Log->info("EventLogWidget::OnReceivedEventLog Event:{}", items.at(i).EventLog.Event);
        AppendEventLog(items.at(i));
    }
}

void EventLogWidget::OnFilterTable(const QVector<QStringList>& filter)
{
    QStringList ColoFilter = filter.at(0);
    QStringList AppFilter = filter.at(1);
    QStringList AccountFilter = filter.at(2);
    QStringList TickerFilter = filter.at(3);
    m_Filter[0] = ColoFilter;
    m_Filter[1] = AppFilter;
    m_Filter[4] = AccountFilter;
    m_Filter[5] = TickerFilter;
    m_XSortFilterProxyModel->setRowFilter(m_Filter);
    m_XSortFilterProxyModel->resetFilter();
}

void EventLogWidget::AppendEventLog(const Message::PackMessage& msg)
{
    FinTechUI::XTableModelRow* ModelRow = new FinTechUI::XTableModelRow;
    FinTechUI::XTableModelItem* ColoItem = new FinTechUI::XTableModelItem(msg.EventLog.Colo);
    ModelRow->push_back(ColoItem);
    FinTechUI::XTableModelItem* AppNameItem = new FinTechUI::XTableModelItem(msg.EventLog.App);
    ModelRow->push_back(AppNameItem);
    FinTechUI::XTableModelItem* BrokerItem = new FinTechUI::XTableModelItem(msg.EventLog.Broker);
    ModelRow->push_back(BrokerItem);
    FinTechUI::XTableModelItem* ProductItem = new FinTechUI::XTableModelItem(msg.EventLog.Product);
    ModelRow->push_back(ProductItem);
    FinTechUI::XTableModelItem* AccountItem = new FinTechUI::XTableModelItem(msg.EventLog.Account);
    ModelRow->push_back(AccountItem);
    FinTechUI::XTableModelItem* TickerItem = new FinTechUI::XTableModelItem(msg.EventLog.Ticker);
    ModelRow->push_back(TickerItem);
    FinTechUI::XTableModelItem* UpdateTimeItem = new FinTechUI::XTableModelItem(msg.EventLog.UpdateTime + 11);
    ModelRow->push_back(UpdateTimeItem);
    FinTechUI::XTableModelItem* EventItem = new FinTechUI::XTableModelItem(msg.EventLog.Event, Qt::AlignLeft | Qt::AlignVCenter);
    ModelRow->push_back(EventItem);
    
    FinTechUI::XTableModel::setRowBackgroundColor(ModelRow, GetEventLogColor(msg.EventLog));
    m_EventLogModel->appendRow(ModelRow);

    bool ok = false;
    QStringList& Apps = m_ColoAppSetMap[msg.EventLog.Colo];
    if(!Apps.contains(msg.EventLog.App))
    {
        Apps.append(msg.EventLog.App);
        ok = true;
    }
    QStringList& Accounts = m_AppAccountSetMap[msg.EventLog.App];
    if(!Accounts.contains(msg.EventLog.Account))
    {
        Accounts.append(msg.EventLog.Account);
        ok = true;
    }
    QStringList& Tickers = m_AccountTickerSetMap[msg.EventLog.Account];
    if(!Tickers.contains(msg.EventLog.Ticker))
    {
        Tickers.append(msg.EventLog.Ticker);
        ok = true;
    }
    if(ok)
    {
        QVector<QMap<QString, QStringList>> data;
        data.append(m_ColoAppSetMap);
        data.append(m_AppAccountSetMap);
        data.append(m_AccountTickerSetMap);
        m_FilterWidget->SetDataRelationMap(data);
    }
}