#include "PermissionWidget.h"

extern Utils::Logger *gLogger;


PermissionWidget::PermissionWidget(QWidget *parent): FinTechUI::TabPageWidget(parent)
{
    m_ControlPannelWidget = new QWidget;
    m_PermissionTableView = new QTableView;
    m_PermissionTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_PermissionTableView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_PermissionTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_PermissionTableView->setSortingEnabled(true);
    m_PermissionTableView->horizontalHeader()->setStretchLastSection(true);
    m_PermissionTableView->verticalHeader()->hide();

    m_PermissionTableModel = new FinTechUI::XTableModel;
    QStringList headers;
    headers << "User" << "PassWord" << "Role" << "UpdateTime" << "Plugins" << "Messages";
    m_PermissionTableModel->setHeaderLabels(headers);
    m_PermissionProxyModel = new FinTechUI::XSortFilterProxyModel;
    m_PermissionProxyModel->setSourceModel(m_PermissionTableModel);
    m_PermissionTableView->setModel(m_PermissionProxyModel);
    m_PermissionProxyModel->setDynamicSortFilter(true);
    m_PermissionTableView->sortByColumn(3, Qt::DescendingOrder);

    int column = 0;
    m_PermissionTableView->setColumnWidth(column++, 80);
    m_PermissionTableView->setColumnWidth(column++, 100);
    m_PermissionTableView->setColumnWidth(column++, 80);
    m_PermissionTableView->setColumnWidth(column++, 200);
    m_PermissionTableView->setColumnWidth(column++, 600);
    m_PermissionTableView->setColumnWidth(column++, 500);

    {
        QVBoxLayout* leftLayout = new QVBoxLayout;
        leftLayout->setContentsMargins(0, 0, 0, 0);
        {
            QGroupBox* UserGroup = new QGroupBox("User");
            QVBoxLayout* vLayout = new QVBoxLayout;
            vLayout->setContentsMargins(0, 0, 0, 0);
            QFormLayout* fromLayout = new QFormLayout;
            fromLayout->setContentsMargins(0, 0, 0, 0);

            QLineEdit* UserNameEdit = new QLineEdit();
            fromLayout->addRow("UserName:", UserNameEdit);

            QLineEdit* PassWDEdit = new QLineEdit;
            fromLayout->addRow("PassWord:", PassWDEdit);

            QComboBox* OperationCombo = new QComboBox;
            OperationCombo->addItems(QStringList() << "Add" << "Update" << "Delete");
            fromLayout->addRow("Operation:", OperationCombo);

            QComboBox* RoleCombo = new QComboBox;
            RoleCombo->addItems(QStringList() << "Monitor" << "Trader" << "Market" << "Risk" << "Admin");
            fromLayout->addRow("Role:", RoleCombo);

            vLayout->addLayout(fromLayout);
            QPushButton* submitButton = new QPushButton("Submit");
            vLayout->addWidget(submitButton);

            connect(m_PermissionTableView, &QTableView::clicked, [ = ](const QModelIndex & index)
            {
                int row = index.row();
                QString UserName = m_PermissionProxyModel->index(row, 0).data().toString();
                QString PassWord = m_PermissionProxyModel->index(row, 1).data().toString();
                QString Role = m_PermissionProxyModel->index(row, 2).data().toString();

                UserNameEdit->setText(UserName);
                PassWDEdit->setText(PassWord);
                RoleCombo->setCurrentText(Role);
            });
            connect(submitButton, &QAbstractButton::clicked,
                    [ = ]()
            {
                QString UserName = UserNameEdit->text();
                QString PassWord = PassWDEdit->text();
                QString Operation = OperationCombo->currentText();
                QString Role = RoleCombo->currentText();
                QString command = QString::asprintf("UserName:%s,PassWord:%s,Operation:%s,Role:%s,Plugins: ,Messages: ",
                                                    UserName.toStdString().c_str(), PassWord.toStdString().c_str(),
                                                    Operation.toStdString().c_str(), Role.toStdString().c_str());
                Message::PackMessage message;
                message.MessageType = Message::EMessageType::ECommand;
                message.Command.CmdType = Message::ECommandType::EUPDATE_USERPERMISSION;
                strncpy(message.Command.Command, command.toStdString().c_str(), sizeof(message.Command.Command));
                HPPackClient::SendData(reinterpret_cast<unsigned char *>(&message), sizeof(message));
            });

            UserGroup->setLayout(vLayout);
            leftLayout->addWidget(UserGroup);
        }

        leftLayout->setSpacing(20);
        leftLayout->addStretch(1);
        m_ControlPannelWidget->setLayout(leftLayout);
    }

    m_Splitter = new QSplitter(Qt::Horizontal);
    m_Splitter->addWidget(m_ControlPannelWidget);
    m_Splitter->addWidget(m_PermissionTableView);

    m_Splitter->setStretchFactor(0, 1);
    m_Splitter->setStretchFactor(1, 7);

    QHBoxLayout* layout = new QHBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_Splitter);
    setLayout(layout);

    connect(m_PermissionTableView, SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT(OnUpdateUserPermission(const QModelIndex&)));
}

void PermissionWidget::UpdateUserPermissionTable(const Message::PackMessage& msg)
{
    QString UserName = msg.LoginResponse.Account;
    auto it = m_PermissionMap.find(UserName);
    if(it == m_PermissionMap.end())
    {
        if(Message::EPermissionOperation::EUSER_DELETE != msg.LoginResponse.Operation)
        {
            FinTechUI::XTableModelRow* ModelRow = new FinTechUI::XTableModelRow;
            FinTechUI::XTableModelItem* AccountItem = new FinTechUI::XTableModelItem(msg.LoginResponse.Account);
            ModelRow->push_back(AccountItem);
            FinTechUI::XTableModelItem* PassWordItem = new FinTechUI::XTableModelItem(msg.LoginResponse.PassWord);
            ModelRow->push_back(PassWordItem);
            FinTechUI::XTableModelItem* RoleItem = new FinTechUI::XTableModelItem(msg.LoginResponse.Role);
            ModelRow->push_back(RoleItem);
            FinTechUI::XTableModelItem* UpdateTimeItem = new FinTechUI::XTableModelItem(msg.LoginResponse.UpdateTime);
            ModelRow->push_back(UpdateTimeItem);
            FinTechUI::XTableModelItem* PluginsItem = new FinTechUI::XTableModelItem(msg.LoginResponse.Plugins, Qt::AlignLeft | Qt::AlignVCenter);
            ModelRow->push_back(PluginsItem);
            FinTechUI::XTableModelItem* MessagesItem = new FinTechUI::XTableModelItem(msg.LoginResponse.Messages, Qt::AlignLeft | Qt::AlignVCenter);
            ModelRow->push_back(MessagesItem);

            m_PermissionTableModel->appendRow(ModelRow);
            m_PermissionMap[UserName] = ModelRow;
            FinTechUI::XTableModel::setRowBackgroundColor(ModelRow, GetUserColor(msg.LoginResponse));
        }
    }
    else
    {
        FinTechUI::XTableModelRow* ModelRow = m_PermissionMap[msg.LoginResponse.Account];
        if(Message::EPermissionOperation::EUSER_DELETE == msg.LoginResponse.Operation)
        {
            m_PermissionTableModel->deleteRow(ModelRow, true);
            m_PermissionMap.erase(it);
        }
        else if(Message::EPermissionOperation::EUSER_UPDATE == msg.LoginResponse.Operation)
        {
            (*ModelRow)[1]->setText(msg.LoginResponse.PassWord);
            (*ModelRow)[2]->setText(msg.LoginResponse.Role);
            (*ModelRow)[3]->setText(msg.LoginResponse.UpdateTime);
            (*ModelRow)[4]->setText(msg.LoginResponse.Plugins);
            (*ModelRow)[5]->setText(msg.LoginResponse.Messages);

            m_PermissionTableModel->updateRow(ModelRow);
            FinTechUI::XTableModel::setRowBackgroundColor(ModelRow, GetUserColor(msg.LoginResponse));
        }
    }
    qDebug() << msg.LoginResponse.Account << msg.LoginResponse.PassWord << msg.LoginResponse.Role << msg.LoginResponse.Operation << msg.LoginResponse.Plugins << msg.LoginResponse.Messages;
}

void PermissionWidget::OnUpdateUserPermission(const QModelIndex &index)
{
    int row = index.row();
    QString UserName = m_PermissionProxyModel->index(row, 0).data().toString();
    QString PassWord = m_PermissionProxyModel->index(row, 1).data().toString();
    QString Role = m_PermissionProxyModel->index(row, 2).data().toString();
    QString Plugin = m_PermissionProxyModel->index(row, 4).data().toString();
    QString Message = m_PermissionProxyModel->index(row, 5).data().toString();
    QStringList Plugins = Plugin.split("|");
    QStringList Messages = Message.split("|");

    QDialog dialog;
    dialog.setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint);
    dialog.setFont(QFont("Times", 12));
    if(index.column() == 4)
    {
        dialog.setFixedSize(400, 400);
        dialog.setWindowTitle("UserPermission");
        QFormLayout* formLayout = new QFormLayout;
        formLayout->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);
        formLayout->setSpacing(15);

        QLabel* UserNameLabel = new QLabel;
        UserNameLabel->setText(UserName);
        formLayout->addRow("UserName:", UserNameLabel);

        QLineEdit* PassWordEdit = new QLineEdit;
        PassWordEdit->setText(PassWord);
        formLayout->addRow("PassWord:", PassWordEdit);

        QComboBox* RoleCombo = new QComboBox;
        RoleCombo->addItems(QStringList() << "Trader" << "Monitor" << "Admin" << "Market");
        RoleCombo->setCurrentText(Role);
        formLayout->addRow("Role:", RoleCombo);

        QVBoxLayout* vLayout = new QVBoxLayout;
        vLayout->setSpacing(6);
        QVector<QCheckBox*> PluginsVec;
        QStringList AllPlugins = QString(PLUGIN_LIST).split("|");
        for (int i = 0; i < AllPlugins.size(); i++)
        {
            QCheckBox* checkBox = new QCheckBox;
            checkBox->setText(AllPlugins.at(i));
            PluginsVec.append(checkBox);
            vLayout->addWidget(checkBox);
            if(Plugins.contains(AllPlugins.at(i)))
            {
                checkBox->setChecked(true);
            }
            else
            {
                checkBox->setChecked(false);
            }
        }
        formLayout->addRow("Plugins:", vLayout);

        QTextEdit* MessageTextEdit = new QTextEdit();
        MessageTextEdit->setText(Message);
        MessageTextEdit->setReadOnly(true);
        MessageTextEdit->setLineWrapMode(QTextEdit::WidgetWidth);
        formLayout->addRow("Messages:", MessageTextEdit);

        QDialogButtonBox* buttonBox = new QDialogButtonBox;
        buttonBox->setCenterButtons(true);
        buttonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
        formLayout->addWidget(buttonBox);
        connect(buttonBox, SIGNAL(accepted()), &dialog, SLOT(accept()));
        connect(buttonBox, SIGNAL(rejected()), &dialog, SLOT(reject()));

        dialog.setLayout(formLayout);

        if(dialog.exec() == QDialog::Accepted)
        {
            PassWord = PassWordEdit->text();
            Role = RoleCombo->currentText();
            Plugin.clear();
            Plugins.clear();
            for (int i = 0; i < PluginsVec.size(); i++)
            {
                if(PluginsVec[i]->isChecked())
                {
                    Plugins.append(PluginsVec[i]->text());
                }
            }
            Plugin = Plugins.join("|");

            QString command = QString::asprintf("UserName:%s,PassWord:%s,Operation:%s,Role:%s,Plugins:%s,Messages:%s",
                                                UserName.toStdString().c_str(), PassWord.toStdString().c_str(), "Update",
                                                Role.toStdString().c_str(), Plugin.toStdString().c_str(), Message.toStdString().c_str());
            Message::PackMessage message;
            message.MessageType = Message::EMessageType::ECommand;
            message.Command.CmdType = Message::ECommandType::EUPDATE_USERPERMISSION;
            strncpy(message.Command.Command, command.toStdString().c_str(), sizeof(message.Command.Command));
            HPPackClient::SendData(reinterpret_cast<unsigned char *>(&message), sizeof(message));
            Utils::gLogger->Log->info("PermissionWidget::OnUpdateUserPermission Command:{}", message.Command.Command);
        }
    }
    else if(index.column() == 5)
    {
        dialog.setFixedSize(400, 500);
        dialog.setWindowTitle("DataPermission");
        QFormLayout* formLayout = new QFormLayout;
        formLayout->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);
        formLayout->setSpacing(15);

        QLabel* UserNameLabel = new QLabel;
        UserNameLabel->setText(UserName);
        formLayout->addRow("UserName:", UserNameLabel);

        QLineEdit* PassWordEdit = new QLineEdit;
        PassWordEdit->setText(PassWord);
        formLayout->addRow("PassWord:", PassWordEdit);

        QComboBox* RoleCombo = new QComboBox;
        RoleCombo->addItems(QStringList() << "Trader" << "Monitor" << "Admin" << "Market");
        RoleCombo->setCurrentText(Role);
        formLayout->addRow("Role:", RoleCombo);

        QTextEdit* PluginTextEdit = new QTextEdit();
        PluginTextEdit->setText(Plugin);
        PluginTextEdit->setReadOnly(true);
        PluginTextEdit->setLineWrapMode(QTextEdit::WidgetWidth);
        formLayout->addRow("Plugins:", PluginTextEdit);

        QVBoxLayout* vLayout = new QVBoxLayout;
        vLayout->setSpacing(6);
        QVector<QCheckBox*> MessagesVec;
        QStringList AllMessages = QString(MESSAGE_LIST).split("|");
        for (int i = 0; i < AllMessages.size(); i++)
        {
            QCheckBox* checkBox = new QCheckBox;
            checkBox->setText(AllMessages.at(i));
            MessagesVec.append(checkBox);
            vLayout->addWidget(checkBox);
            if(Messages.contains(AllMessages.at(i)))
            {
                checkBox->setChecked(true);
            }
            else
            {
                checkBox->setChecked(false);
            }
        }
        formLayout->addRow("Messages:", vLayout);

        QDialogButtonBox* buttonBox = new QDialogButtonBox;
        buttonBox->setCenterButtons(true);
        buttonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
        formLayout->addWidget(buttonBox);
        connect(buttonBox, SIGNAL(accepted()), &dialog, SLOT(accept()));
        connect(buttonBox, SIGNAL(rejected()), &dialog, SLOT(reject()));

        dialog.setLayout(formLayout);

        if(dialog.exec() == QDialog::Accepted)
        {
            PassWord = PassWordEdit->text();
            Role = RoleCombo->currentText();
            Message.clear();
            Messages.clear();
            for (int i = 0; i < MessagesVec.size(); i++)
            {
                if(MessagesVec[i]->isChecked())
                {
                    Messages.append(MessagesVec[i]->text());
                }
            }
            Message = Messages.join("|");

            QString command = QString::asprintf("UserName:%s,PassWord:%s,Operation:%s,Role:%s,Plugins:%s,Messages:%s",
                                                UserName.toStdString().c_str(), PassWord.toStdString().c_str(), "Update",
                                                Role.toStdString().c_str(), Plugin.toStdString().c_str(),Message.toStdString().c_str());
            Message::PackMessage message;
            message.MessageType = Message::EMessageType::ECommand;
            message.Command.CmdType = Message::ECommandType::EUPDATE_USERPERMISSION;
            strncpy(message.Command.Command, command.toStdString().c_str(), sizeof(message.Command.Command));
            HPPackClient::SendData(reinterpret_cast<unsigned char *>(&message), sizeof(message));
            Utils::gLogger->Log->info("PermissionWidget::OnUpdateUserPermission Command:{}", message.Command.Command);
        }
    }
}
