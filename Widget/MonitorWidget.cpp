#include "MonitorWidget.h"
extern Utils::Logger *gLogger;

MonitorWidget::MonitorWidget(QWidget *parent) : FinTechUI::TabPageWidget(parent)
{
    m_SelectedColoRow = -1;
    InitFilterWidget();
    InitColoStatusTableView();
    InitAppStatusTableView();

    m_RightSplitter = new QSplitter(Qt::Vertical);
    m_RightSplitter->setHandleWidth(0);
    m_RightSplitter->addWidget(m_ColoStatusTableView);
    m_RightSplitter->addWidget(m_AppStatusTableView);

    m_RightSplitter->setStretchFactor(0, 3);
    m_RightSplitter->setStretchFactor(1, 7);
    m_RightSplitter->setCollapsible(0, false);
    m_RightSplitter->setCollapsible(1, false);

    m_Splitter = new QSplitter(Qt::Horizontal);
    m_Splitter->addWidget(m_FilterWidget);
    m_Splitter->addWidget(m_RightSplitter);
    m_Splitter->setStretchFactor(0, 2);
    m_Splitter->setStretchFactor(1, 8);
    m_Splitter->setCollapsible(0, false);

    m_HBoxLayout = new QHBoxLayout;
    m_HBoxLayout->setContentsMargins(0, 0, 0, 0);
    m_HBoxLayout->addWidget(m_Splitter);
    setLayout(m_HBoxLayout);

    connect(m_FilterWidget, &FinTechUI::FilterWidget::FilterChanged, this, &MonitorWidget::OnFilterTable, Qt::UniqueConnection);
}

void MonitorWidget::InitFilterWidget()
{
    m_FilterWidget = new FinTechUI::FilterWidget;
    QStringList filter;
    filter << "Colo" <<  "App";
    m_FilterWidget->SetHeaderLabels(filter);
    m_FilterWidget->SetColumnWidth("Colo", 70);
    m_FilterWidget->SetColumnWidth("App", 140);
}

void MonitorWidget::InitColoStatusTableView()
{
    QStringList headerLabels;
    QList<QVariantMap> headerData;
    {
        QVariantMap data;
        data["name"] = "Colo";
        QStringList items;
        items.append("Colo");
        data["items"] = items;
        headerData.append(data);
        headerLabels.append(items);
    }
    {
        QVariantMap data;
        data["name"] = "OSVersion";
        QStringList items;
        items.append("OSVersion");
        data["items"] = items;
        headerData.append(data);
        headerLabels.append(items);
    }
    
    {
        QVariantMap data;
        data["name"] = "KernelVersion";
        QStringList items;
        items.append("KernelVersion");
        data["items"] = items;
        headerData.append(data);
        headerLabels.append(items);
    }
    {
        QVariantMap data;
        data["name"] = "LoadAverage";
        QStringList items;
        items.append("min1");
        items.append("min5");
        items.append("min15");
        items.append("CPUS");
        data["items"] = items;
        headerData.append(data);
        headerLabels.append(items);
    }

    {
        QVariantMap data;
        data["name"] = "CPUUsage";
        QStringList items;
        items.append("UserRate");
        items.append("SysRate");
        items.append("IdleRate");
        items.append("IOWaitRate");
        items.append("IrqRate");
        items.append("SoftIrqRate");
        items.append("UsedRate");
        data["items"] = items;
        headerData.append(data);
        headerLabels.append(items);
    }

    {
        QVariantMap data;
        data["name"] = "MemoryInfo";
        QStringList items;
        items.append("Total");
        items.append("Free");
        items.append("UsedRate");
        data["items"] = items;
        headerData.append(data);
        headerLabels.append(items);
    }

    {
        QVariantMap data;
        data["name"] = "DiskInfo";
        QStringList items;
        items.append("Total");
        items.append("Free");
        items.append("UsedRate");
        items.append("/");
        items.append("/home/");
        data["items"] = items;
        headerData.append(data);
        headerLabels.append(items);
    }
    {
        QVariantMap data;
        data["name"] = "UpdateTime";
        QStringList items;
        items.append("UpdateTime");
        data["items"] = items;
        headerData.append(data);
        headerLabels.append(items);
    }

    FinTechUI::XHeaderView* hHeader = new FinTechUI::XHeaderView(headerData);
    int column = 0;
    hHeader->setColumnWidth(column++, 70);
    hHeader->setColumnWidth(column++, 110);
    hHeader->setColumnWidth(column++, 180);
    hHeader->setColumnWidth(column++, 50);
    hHeader->setColumnWidth(column++, 50);
    hHeader->setColumnWidth(column++, 50);
    hHeader->setColumnWidth(column++, 50);
    hHeader->setColumnWidth(column++, 70);
    hHeader->setColumnWidth(column++, 70);
    hHeader->setColumnWidth(column++, 70);
    hHeader->setColumnWidth(column++, 70);
    hHeader->setColumnWidth(column++, 70);
    hHeader->setColumnWidth(column++, 70);
    hHeader->setColumnWidth(column++, 70);
    hHeader->setColumnWidth(column++, 80);
    hHeader->setColumnWidth(column++, 80);
    hHeader->setColumnWidth(column++, 80);
    hHeader->setColumnWidth(column++, 80);
    hHeader->setColumnWidth(column++, 80);
    hHeader->setColumnWidth(column++, 80);
    hHeader->setColumnWidth(column++, 60);
    hHeader->setColumnWidth(column++, 60);
    hHeader->setColumnWidth(column++, 120);
    m_ColoStatusTableView = new QTableView(this);
    m_ColoStatusTableView->setObjectName("ColoStatusTable");
    m_ColoStatusTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_ColoStatusTableView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_ColoStatusTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_ColoStatusTableView->setSortingEnabled(true);
    m_ColoStatusTableView->sortByColumn(0, Qt::AscendingOrder);
    m_ColoStatusTableView->horizontalHeader()->setStretchLastSection(true);
    m_ColoStatusTableView->verticalHeader()->hide();
    m_ColoStatusTableView->show();
    
    m_ColoStatusTableModel = new FinTechUI::XTableModel;
    m_ColoStatusTableModel->setHeaderLabels(headerLabels);
    m_ColoStatusProxyModel = new FinTechUI::XSortFilterProxyModel;
    m_ColoStatusProxyModel->setSourceModel(m_ColoStatusTableModel);
    m_ColoStatusTableView->setModel(m_ColoStatusProxyModel);
    // set Horizontal Header
    m_ColoStatusTableView->setHorizontalHeader(hHeader);

    m_ColoStatusProxyModel->setDynamicSortFilter(true);
    m_ColoStatusTableView->sortByColumn(6, Qt::DescendingOrder);

    connect(m_ColoStatusTableView, SIGNAL(clicked(const QModelIndex&)), this, SLOT(OnSelectColoRow(const QModelIndex&)));
}

void MonitorWidget::InitAppStatusTableView()
{
    m_AppStatusTableView = new QTableView(this);
    m_AppStatusTableView->setObjectName("AppStatusTable");
    m_AppStatusTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_AppStatusTableView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_AppStatusTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_AppStatusTableView->setSortingEnabled(true);
    m_AppStatusTableView->horizontalHeader()->setStretchLastSection(true);
    m_AppStatusTableView->verticalHeader()->hide();

    m_AppStatusTableModel = new FinTechUI::XTableModel;
    QStringList headerData;
    headerData << "Colo" << "AppName" << "Account" << "PID" << "Status" << "KillApp" << "StartApp" << "UsedCPURate" << "UsedMemSize" 
               << "StartTime" << "LastStartTime" << "CommitID" << "UtilsCommitID" << "APIVersion" << "UpdateTime" << "StartScript";
    m_AppStatusTableModel->setHeaderLabels(headerData);
    m_AppStatusProxyModel = new FinTechUI::XSortFilterProxyModel;
    m_AppStatusProxyModel->setSourceModel(m_AppStatusTableModel);
    m_AppStatusTableView->setModel(m_AppStatusProxyModel);
    m_AppStatusProxyModel->setDynamicSortFilter(true);
    m_AppStatusTableView->sortByColumn(9, Qt::DescendingOrder);

    FinTechUI::XButtonDelegate* StopButtonDelegate = new FinTechUI::XButtonDelegate("Stop");
    FinTechUI::XButtonDelegate* StartButtonDelegate = new FinTechUI::XButtonDelegate("Start");
    m_AppStatusTableView->setItemDelegateForColumn(5, StopButtonDelegate);
    m_AppStatusTableView->setItemDelegateForColumn(6, StartButtonDelegate);

    connect(StopButtonDelegate, SIGNAL(Clicked(const QModelIndex&)), this, SLOT(OnKillAppButtonClicked(const QModelIndex&)), Qt::UniqueConnection);
    connect(StartButtonDelegate, SIGNAL(Clicked(const QModelIndex&)), this, SLOT(OnStartAppButtonClicked(const QModelIndex&)), Qt::UniqueConnection);

    int column = 0;
    m_AppStatusTableView->setColumnWidth(column++, 70);
    m_AppStatusTableView->setColumnWidth(column++, 140);
    m_AppStatusTableView->setColumnWidth(column++, 100);
    m_AppStatusTableView->setColumnWidth(column++, 60);
    m_AppStatusTableView->setColumnWidth(column++, 80);
    m_AppStatusTableView->setColumnWidth(column++, 70);
    m_AppStatusTableView->setColumnWidth(column++, 70);
    m_AppStatusTableView->setColumnWidth(column++, 100);
    m_AppStatusTableView->setColumnWidth(column++, 100);
    m_AppStatusTableView->setColumnWidth(column++, 120);
    m_AppStatusTableView->setColumnWidth(column++, 120);
    m_AppStatusTableView->setColumnWidth(column++, 120);
    m_AppStatusTableView->setColumnWidth(column++, 120);
    m_AppStatusTableView->setColumnWidth(column++, 90);
    m_AppStatusTableView->setColumnWidth(column++, 120);
    m_AppStatusTableView->setColumnWidth(column++, 1500);
}

void MonitorWidget::UpdateColoStatusTable(const Message::PackMessage& msg)
{
    QMap<QString, FinTechUI::XTableModelRow*>::iterator it =  m_ColoStatusMap.find(msg.ColoStatus.Colo);
    if(it != m_ColoStatusMap.end())
    {
        UpdateRow(msg.ColoStatus);
    }
    else
    {
        AppendRow(msg.ColoStatus);
    }
}

void MonitorWidget::AppendRow(const Message::TColoStatus& ColoStatus)
{
    FinTechUI::XTableModelRow* ModelRow = new FinTechUI::XTableModelRow;
    FinTechUI::XTableModelItem* ColoItem = new FinTechUI::XTableModelItem(ColoStatus.Colo);
    ModelRow->push_back(ColoItem);
    FinTechUI::XTableModelItem* OSVersionItem = new FinTechUI::XTableModelItem(ColoStatus.OSVersion);
    ModelRow->push_back(OSVersionItem);
    FinTechUI::XTableModelItem* KernelVersionItem = new FinTechUI::XTableModelItem(ColoStatus.KernelVersion);
    ModelRow->push_back(KernelVersionItem);

    FinTechUI::XTableModelItem* Min1Item = new FinTechUI::XTableModelItem(ColoStatus.LoadAverage.Min1);
    ModelRow->push_back(Min1Item);
    FinTechUI::XTableModelItem* Min5Item = new FinTechUI::XTableModelItem(ColoStatus.LoadAverage.Min5);
    ModelRow->push_back(Min5Item);
    FinTechUI::XTableModelItem* Min15Item = new FinTechUI::XTableModelItem(ColoStatus.LoadAverage.Min15);
    ModelRow->push_back(Min15Item);
    FinTechUI::XTableModelItem* CPUSItem = new FinTechUI::XTableModelItem(ColoStatus.LoadAverage.CPUS);
    ModelRow->push_back(CPUSItem);

    char buffer[32] = {0};
    sprintf(buffer, "%.2f%%", ColoStatus.CPUUsage.UserRate * 100);
    FinTechUI::XTableModelItem* UserRateItem = new FinTechUI::XTableModelItem(buffer);
    ModelRow->push_back(UserRateItem);
    sprintf(buffer, "%.2f%%", ColoStatus.CPUUsage.SysRate * 100);
    FinTechUI::XTableModelItem* SysRateItem = new FinTechUI::XTableModelItem(buffer);
    ModelRow->push_back(SysRateItem);
    sprintf(buffer, "%.2f%%", ColoStatus.CPUUsage.IdleRate * 100);
    FinTechUI::XTableModelItem* IdleRateItem = new FinTechUI::XTableModelItem(buffer);
    ModelRow->push_back(IdleRateItem);
    sprintf(buffer, "%.2f%%", ColoStatus.CPUUsage.IOWaitRate * 100);
    FinTechUI::XTableModelItem* IOWaitRateItem = new FinTechUI::XTableModelItem(buffer);
    ModelRow->push_back(IOWaitRateItem);
    sprintf(buffer, "%.2f%%", ColoStatus.CPUUsage.IrqRate * 100);
    FinTechUI::XTableModelItem* IrqRateItem = new FinTechUI::XTableModelItem(buffer);
    ModelRow->push_back(IrqRateItem);
    sprintf(buffer, "%.2f%%", ColoStatus.CPUUsage.SoftIrqRate * 100);
    FinTechUI::XTableModelItem* SoftIrqRateItem = new FinTechUI::XTableModelItem(buffer);
    ModelRow->push_back(SoftIrqRateItem);
    sprintf(buffer, "%.2f%%", ColoStatus.CPUUsage.UsedRate * 100);
    FinTechUI::XTableModelItem* UsedRateItem = new FinTechUI::XTableModelItem(buffer);
    ModelRow->push_back(UsedRateItem);

    sprintf(buffer, "%.2fGB", ColoStatus.MemoryInfo.Total);
    FinTechUI::XTableModelItem* MemoryInfoTotalItem = new FinTechUI::XTableModelItem(buffer);
    ModelRow->push_back(MemoryInfoTotalItem);
    sprintf(buffer, "%.2fGB", ColoStatus.MemoryInfo.Free);
    FinTechUI::XTableModelItem* MemoryInfoFreeItem = new FinTechUI::XTableModelItem(buffer);
    ModelRow->push_back(MemoryInfoFreeItem);
    sprintf(buffer, "%.2f%%", ColoStatus.MemoryInfo.UsedRate * 100);
    FinTechUI::XTableModelItem* MemoryInfoUsedRateItem = new FinTechUI::XTableModelItem(buffer);
    ModelRow->push_back(MemoryInfoUsedRateItem);

    sprintf(buffer, "%.2fGB", ColoStatus.DiskInfo.Total);
    FinTechUI::XTableModelItem* DiskInfoTotalItem = new FinTechUI::XTableModelItem(buffer);
    ModelRow->push_back(DiskInfoTotalItem);
    sprintf(buffer, "%.2fGB", ColoStatus.DiskInfo.Free);
    FinTechUI::XTableModelItem* DiskInfoFreeItem = new FinTechUI::XTableModelItem(buffer);
    ModelRow->push_back(DiskInfoFreeItem);
    sprintf(buffer, "%.2f%%", ColoStatus.DiskInfo.UsedRate * 100);
    FinTechUI::XTableModelItem* DiskInfoUsedRateItem = new FinTechUI::XTableModelItem(buffer);
    ModelRow->push_back(DiskInfoUsedRateItem);
    sprintf(buffer, "%.2f%%", ColoStatus.DiskInfo.Mount1UsedRate * 100);
    FinTechUI::XTableModelItem* DiskInfoMount1UsedRateItem = new FinTechUI::XTableModelItem(buffer);
    ModelRow->push_back(DiskInfoMount1UsedRateItem);
    sprintf(buffer, "%.2f%%", ColoStatus.DiskInfo.Mount2UsedRate * 100);
    FinTechUI::XTableModelItem* DiskInfoMount2UsedRateItem = new FinTechUI::XTableModelItem(buffer);
    ModelRow->push_back(DiskInfoMount2UsedRateItem);
    FinTechUI::XTableModelItem* UpdateTimeItem = new FinTechUI::XTableModelItem(ColoStatus.UpdateTime + 11, Qt::AlignLeft | Qt::AlignVCenter);
    ModelRow->push_back(UpdateTimeItem);

    FinTechUI::XTableModel::setRowBackgroundColor(ModelRow, GetColoStatusColor(ColoStatus));
    m_ColoStatusTableModel->appendRow(ModelRow);
    m_ColoStatusMap[ColoStatus.Colo] = ModelRow;
}

void MonitorWidget::UpdateRow(const Message::TColoStatus& ColoStatus)
{
    FinTechUI::XTableModelRow* ModelRow = m_ColoStatusMap[ColoStatus.Colo];
    int column = 3;
    if(ColoStatus.LoadAverage.Min1 > ColoStatus.LoadAverage.CPUS * 0.8)
    {
        (*ModelRow)[column]->setForeground(Qt::red);
    }
    else
    {
        (*ModelRow)[column]->setForeground(Qt::black);
    }
    (*ModelRow)[column++]->setText(ColoStatus.LoadAverage.Min1);
    if(ColoStatus.LoadAverage.Min5 > ColoStatus.LoadAverage.CPUS * 0.8)
    {
        (*ModelRow)[column]->setForeground(Qt::red);
    }
    else
    {
        (*ModelRow)[column]->setForeground(Qt::black);
    }
    (*ModelRow)[column++]->setText(ColoStatus.LoadAverage.Min5);
    if(ColoStatus.LoadAverage.Min15 > ColoStatus.LoadAverage.CPUS * 0.8)
    {
        (*ModelRow)[column]->setForeground(Qt::red);
    }
    else
    {
        (*ModelRow)[column]->setForeground(Qt::black);
    }
    (*ModelRow)[column++]->setText(ColoStatus.LoadAverage.Min15);
    (*ModelRow)[column++]->setText(ColoStatus.LoadAverage.CPUS);

    char buffer[32] = {0};
    sprintf(buffer, "%.2f%%", ColoStatus.CPUUsage.UserRate * 100);
    (*ModelRow)[column++]->setText(buffer);
    sprintf(buffer, "%.2f%%", ColoStatus.CPUUsage.SysRate * 100);
    (*ModelRow)[column++]->setText(buffer);
    sprintf(buffer, "%.2f%%", ColoStatus.CPUUsage.IdleRate * 100);
    (*ModelRow)[column++]->setText(buffer);
    sprintf(buffer, "%.2f%%", ColoStatus.CPUUsage.IOWaitRate * 100);
    if(ColoStatus.CPUUsage.IOWaitRate > 0.1)
    {
        (*ModelRow)[column]->setForeground(Qt::red);
    }
    else
    {
        (*ModelRow)[column]->setForeground(Qt::black);
    }
    (*ModelRow)[column++]->setText(buffer);
    sprintf(buffer, "%.2f%%", ColoStatus.CPUUsage.IrqRate * 100);
    if(ColoStatus.CPUUsage.IrqRate > 0.1)
    {
        (*ModelRow)[column]->setForeground(Qt::red);
    }
    else
    {
        (*ModelRow)[column]->setForeground(Qt::black);
    }
    (*ModelRow)[column++]->setText(buffer);
    sprintf(buffer, "%.2f%%", ColoStatus.CPUUsage.SoftIrqRate * 100);
    if(ColoStatus.CPUUsage.SoftIrqRate > 0.1)
    {
        (*ModelRow)[column]->setForeground(Qt::red);
    }
    else
    {
        (*ModelRow)[column]->setForeground(Qt::black);
    }
    (*ModelRow)[column++]->setText(buffer);
    sprintf(buffer, "%.2f%%", ColoStatus.CPUUsage.UsedRate * 100);
    if(ColoStatus.CPUUsage.UsedRate > 0.8)
    {
        (*ModelRow)[column]->setForeground(Qt::red);
    }
    else
    {
        (*ModelRow)[column]->setForeground(Qt::black);
    }
    (*ModelRow)[column++]->setText(buffer);

    sprintf(buffer, "%.2fGB", ColoStatus.MemoryInfo.Total);
    (*ModelRow)[column++]->setText(buffer);
    sprintf(buffer, "%.2fGB", ColoStatus.MemoryInfo.Free);
    (*ModelRow)[column++]->setText(buffer);
    sprintf(buffer, "%.2f%%", ColoStatus.MemoryInfo.UsedRate * 100);
    if(ColoStatus.MemoryInfo.UsedRate > 0.8)
    {
        (*ModelRow)[column]->setForeground(Qt::red);
    }
    else
    {
        (*ModelRow)[column]->setForeground(Qt::black);
    }
    (*ModelRow)[column++]->setText(buffer);

    sprintf(buffer, "%.2fGB", ColoStatus.DiskInfo.Total);
    (*ModelRow)[column++]->setText(buffer);
    sprintf(buffer, "%.2fGB", ColoStatus.DiskInfo.Free);
    (*ModelRow)[column++]->setText(buffer);
    sprintf(buffer, "%.2f%%", ColoStatus.DiskInfo.UsedRate * 100);
    if(ColoStatus.DiskInfo.UsedRate > 0.8)
    {
        (*ModelRow)[column]->setForeground(Qt::red);
    }
    else
    {
        (*ModelRow)[column]->setForeground(Qt::black);
    }
    (*ModelRow)[column++]->setText(buffer);
    sprintf(buffer, "%.2f%%", ColoStatus.DiskInfo.Mount1UsedRate * 100);
    if(ColoStatus.DiskInfo.Mount1UsedRate > 0.8)
    {
        (*ModelRow)[column]->setForeground(Qt::red);
    }
    else
    {
        (*ModelRow)[column]->setForeground(Qt::black);
    }
    (*ModelRow)[column++]->setText(buffer);
    sprintf(buffer, "%.2f%%", ColoStatus.DiskInfo.Mount2UsedRate * 100);
    if(ColoStatus.DiskInfo.Mount2UsedRate > 0.8)
    {
        (*ModelRow)[column]->setForeground(Qt::red);
    }
    else
    {
        (*ModelRow)[column]->setForeground(Qt::black);
    }
    (*ModelRow)[column++]->setText(buffer);
    (*ModelRow)[column++]->setText(ColoStatus.UpdateTime + 11);

    FinTechUI::XTableModel::setRowBackgroundColor(ModelRow, GetColoStatusColor(ColoStatus));
    m_ColoStatusTableModel->updateRow(ModelRow);
}

void MonitorWidget::UpdateAppStatusTable(const Message::PackMessage& msg)
{
    QString Key = QString(msg.AppStatus.Colo) + ":" + msg.AppStatus.AppName + ":" + msg.AppStatus.Account;
    QMap<QString, FinTechUI::XTableModelRow*>::iterator it =  m_AppStatusMap.find(Key);
    if(it != m_AppStatusMap.end())
    {
        UpdateRow(msg.AppStatus);
        memcpy(&m_AppStatusMessageMap[Key], &msg, sizeof(m_AppStatusMessageMap[Key]));
    }
    else
    {
        AppendRow(msg.AppStatus);
        m_AppStatusMessageMap[Key] = msg;
    }
}

void MonitorWidget::AppendRow(const Message::TAppStatus& AppStatus)
{
    FinTechUI::XTableModelRow* ModelRow = new FinTechUI::XTableModelRow;
    FinTechUI::XTableModelItem* ColoItem = new FinTechUI::XTableModelItem(AppStatus.Colo);
    ModelRow->push_back(ColoItem);
    FinTechUI::XTableModelItem* AppNameItem = new FinTechUI::XTableModelItem(AppStatus.AppName);
    ModelRow->push_back(AppNameItem);
    FinTechUI::XTableModelItem* AccountItem = new FinTechUI::XTableModelItem(AppStatus.Account);
    ModelRow->push_back(AccountItem);
    FinTechUI::XTableModelItem* PIDItem = new FinTechUI::XTableModelItem(AppStatus.PID);
    ModelRow->push_back(PIDItem);
    FinTechUI::XTableModelItem* StatusItem = new FinTechUI::XTableModelItem(AppStatus.Status);
    ModelRow->push_back(StatusItem);

    FinTechUI::XTableModelItem* StopItem = new FinTechUI::XTableModelItem("");
    ModelRow->push_back(StopItem);
    FinTechUI::XTableModelItem* StartItem = new FinTechUI::XTableModelItem("");
    ModelRow->push_back(StartItem);

    char buffer[32] = {0};
    sprintf(buffer, "%.2f%%", AppStatus.UsedCPURate * 100);
    FinTechUI::XTableModelItem* UsedCPURateItem = new FinTechUI::XTableModelItem(buffer);
    ModelRow->push_back(UsedCPURateItem);
    sprintf(buffer, "%.2fMB", AppStatus.UsedMemSize * 100);
    FinTechUI::XTableModelItem* UsedMemSizeItem = new FinTechUI::XTableModelItem(buffer);
    ModelRow->push_back(UsedMemSizeItem);
    FinTechUI::XTableModelItem* StartTimeItem = new FinTechUI::XTableModelItem(AppStatus.StartTime + 11);
    ModelRow->push_back(StartTimeItem);
    FinTechUI::XTableModelItem* LastStartTimeItem = new FinTechUI::XTableModelItem(AppStatus.LastStartTime + 11);
    ModelRow->push_back(LastStartTimeItem);
    FinTechUI::XTableModelItem* CommitIDItem = new FinTechUI::XTableModelItem(AppStatus.CommitID);
    ModelRow->push_back(CommitIDItem);
    FinTechUI::XTableModelItem* UtilsCommitIDItem = new FinTechUI::XTableModelItem(AppStatus.UtilsCommitID);
    ModelRow->push_back(UtilsCommitIDItem);
    FinTechUI::XTableModelItem* APIVersionItem = new FinTechUI::XTableModelItem(AppStatus.APIVersion);
    ModelRow->push_back(APIVersionItem);
    FinTechUI::XTableModelItem* UpdateTimeItem = new FinTechUI::XTableModelItem(AppStatus.UpdateTime + 11);
    ModelRow->push_back(UpdateTimeItem);
    FinTechUI::XTableModelItem* StartScriptItem = new FinTechUI::XTableModelItem(AppStatus.StartScript, Qt::AlignLeft | Qt::AlignVCenter);
    ModelRow->push_back(StartScriptItem);

    FinTechUI::XTableModel::setRowBackgroundColor(ModelRow, GetAppStatusColor(AppStatus));
    m_AppStatusTableModel->appendRow(ModelRow);
    
    QString Key = QString(AppStatus.Colo) + ":" + AppStatus.AppName + ":" + AppStatus.Account;
    m_AppStatusMap[Key] = ModelRow;

    bool ok = false;
    QStringList& Apps = m_ColoAppSetMap[AppStatus.Colo];
    QString AppName = QString(AppStatus.AppName);
    if(!Apps.contains(AppName))
    {
        Apps.append(AppName);
        ok = true;
    }
    if(ok)
    {
        QVector<QMap<QString, QStringList>> data;
        data.append(m_ColoAppSetMap);
        m_FilterWidget->SetDataRelationMap(data);
    }
}

void MonitorWidget::UpdateRow(const Message::TAppStatus& AppStatus)
{
    QString Key = QString(AppStatus.Colo) + ":" + AppStatus.AppName + ":" + AppStatus.Account;
    FinTechUI::XTableModelRow* ModelRow = m_AppStatusMap[Key];
    int column = 3;
    (*ModelRow)[column++]->setText(AppStatus.PID);
    (*ModelRow)[column++]->setText(AppStatus.Status);
    column += 2;
    char buffer[32] = {0};
    sprintf(buffer, "%.2f%%", AppStatus.UsedCPURate * 100);
    (*ModelRow)[column++]->setText(buffer);
    sprintf(buffer, "%.2fMB", AppStatus.UsedMemSize);
    (*ModelRow)[column++]->setText(buffer);
    (*ModelRow)[column++]->setText(AppStatus.StartTime + 11);
    (*ModelRow)[column++]->setText(AppStatus.LastStartTime + 11);
    (*ModelRow)[column++]->setText(AppStatus.CommitID);
    (*ModelRow)[column++]->setText(AppStatus.UtilsCommitID);
    (*ModelRow)[column++]->setText(AppStatus.APIVersion);
    (*ModelRow)[column++]->setText(AppStatus.UpdateTime + 11);
    (*ModelRow)[column++]->setText(AppStatus.StartScript);

    FinTechUI::XTableModel::setRowBackgroundColor(ModelRow, GetAppStatusColor(AppStatus));
    m_AppStatusTableModel->updateRow(ModelRow);
    
}

void MonitorWidget::KillApp(const Message::TAppStatus& AppStatus)
{
    Message::PackMessage Command;
    Command.MessageType = Message::EMessageType::ECommand;
    Command.Command.CmdType = Message::ECommandType::EKILL_APP;
    strncpy(Command.Command.Colo, AppStatus.Colo, sizeof(Command.Command.Colo));
    strncpy(Command.Command.Account, AppStatus.Account, sizeof(Command.Command.Account));
    char cmd[128] = {0};
    sprintf(cmd, "sudo kill -9 %d", AppStatus.PID);
    strncpy(Command.Command.Command, cmd, sizeof(Command.Command.Command));
    HPPackClient::SendData((const unsigned char *)&Command, sizeof(Command));
    Utils::gLogger->Log->info("MonitorWidget::KillApp Colo:{} Account:{} Command:{}", Command.Command.Colo, Command.Command.Account, Command.Command.Command);
}

void MonitorWidget::StartApp(const Message::TAppStatus& AppStatus)
{
    Message::PackMessage Command;
    Command.MessageType = Message::EMessageType::ECommand;
    Command.Command.CmdType = Message::ECommandType::ESTART_APP;
    strncpy(Command.Command.Colo, AppStatus.Colo, sizeof(Command.Command.Colo));
    strncpy(Command.Command.Account, AppStatus.Account, sizeof(Command.Command.Account));
    strncpy(Command.Command.Command, AppStatus.StartScript, sizeof(Command.Command.Command));
    HPPackClient::SendData((const unsigned char *)&Command, sizeof(Command));
    Utils::gLogger->Log->info("MonitorWidget::StartApp Colo:{} Account:{} Command:{}", Command.Command.Colo, Command.Command.Account, Command.Command.Command);
}

void MonitorWidget::OnReceivedAppReport(const QList<Message::PackMessage>& items)
{
    for(int i = 0; i < items.size(); i++)
    {
        if(items.at(i).MessageType == Message::EMessageType::EAppStatus)
        {
            UpdateAppStatusTable(items.at(i));
            Utils::gLogger->Log->info("MonitorWidget::OnReceivedAppReport AppName:{}", items.at(i).AppStatus.AppName);
        }
        else if(items.at(i).MessageType == Message::EMessageType::EColoStatus)
        {
            UpdateColoStatusTable(items.at(i));
            Utils::gLogger->Log->info("MonitorWidget::OnReceivedAppReport Colo:{}", items.at(i).ColoStatus.Colo);
        }
    }
}


void MonitorWidget::OnKillAppButtonClicked(const QModelIndex& index)
{
    int row = index.row();
    QString Colo = m_AppStatusProxyModel->index(row, 0).data().toString();
    QString AppName = m_AppStatusProxyModel->index(row, 1).data().toString();
    if(!AppName.contains("XWatcher"))
    {
        QString Account = m_AppStatusProxyModel->index(row, 2).data().toString();
        QString PID = m_AppStatusProxyModel->index(row, 3).data().toString();
        QString Key = Colo + ":" + AppName + ":" + Account;
        char buffer[128] = {0};
        sprintf(buffer, "Kill %s Account:%s PID:%d On %s?", AppName.toStdString().c_str(), Account.toStdString().c_str() , PID.toInt(), Colo.toStdString().c_str());
        if(QMessageBox::Yes == QMessageBox::question(this, "Kill App: " + AppName, buffer))
        {
            KillApp(m_AppStatusMessageMap[Key].AppStatus);
        }
    }
}

void MonitorWidget::OnStartAppButtonClicked(const QModelIndex& index)
{
    int row = index.row();
    QString Status = m_AppStatusProxyModel->index(row, 4).data().toString();
    if(Status.contains("Stoped"))
    {
        QString Colo = m_AppStatusProxyModel->index(row, 0).data().toString();
        QString AppName = m_AppStatusProxyModel->index(row, 1).data().toString();
        if(!AppName.contains("XWatcher"))
        {
            QString Account = m_AppStatusProxyModel->index(row, 2).data().toString();
            QString Key = Colo + ":" + AppName + ":" + Account;
            char buffer[128] = {0};
            sprintf(buffer, "Start %s Account:%s On %s?", AppName.toStdString().c_str(), Account.toStdString().c_str(), Colo.toStdString().c_str());
            if(QMessageBox::Yes == QMessageBox::question(this, "Start App: " + AppName, buffer))
            {
                StartApp(m_AppStatusMessageMap[Key].AppStatus);
            }
        }
    }
}

void MonitorWidget::OnSelectColoRow(const QModelIndex& index)
{
    int row = index.row();
    if(m_SelectedColoRow == row)
    {
        m_ColoStatusTableView->clearSelection();
        m_SelectedColoRow = -1;
    }
    else
    {
        m_SelectedColoRow = row;
    }
    if(m_SelectedColoRow > -1)
    {
        QString Colo = m_ColoStatusProxyModel->index(m_SelectedColoRow, 0).data().toString();
        QStringList ColoFilter;
        ColoFilter << Colo;
        m_Filter[0] = ColoFilter;
        m_AppStatusProxyModel->setRowFilter(m_Filter);
        m_AppStatusProxyModel->resetFilter();
        m_Filter.clear();
    }
    else
    {
        m_Filter.clear();
        m_AppStatusProxyModel->setRowFilter(m_Filter);
        m_AppStatusProxyModel->resetFilter();
        m_Filter.clear();
    }
}

void MonitorWidget::OnFilterTable(const QVector<QStringList>& filter)
{
    QStringList ColoFilter = filter.at(0);
    QStringList AppFilter = filter.at(1);
    m_Filter[0] = ColoFilter;
    m_Filter[1] = AppFilter;
    m_AppStatusProxyModel->setRowFilter(m_Filter);
    m_AppStatusProxyModel->resetFilter();
    m_Filter.clear();

    m_Filter[0] = ColoFilter;
    m_ColoStatusProxyModel->setRowFilter(m_Filter);
    m_ColoStatusProxyModel->resetFilter();
    m_Filter.clear();
}
