#ifndef PERMISSIONWIDGET_H
#define PERMISSIONWIDGET_H

#include <QObject>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QFormLayout>
#include <QComboBox>
#include <QTimerEvent>
#include <QHeaderView>
#include <QSplitter>
#include <QPushButton>
#include <QDialog>
#include <QLineEdit>
#include <QTextEdit>
#include <QMultiMap>
#include <QDialogButtonBox>
#include <QQueue>
#include <QWidget>
#include <QLabel>
#include <QDebug>
#include <QCheckBox>
#include <QGroupBox>
#include <QVariant>
#include <QVariantMap>
#include <QTableWidget>
#include <QSet>
#include <QColor>
#include <QTableWidgetItem>
#include "Logger.h"
#include "PackMessage.hpp"
#include "FinTechUI/DragTabWidget/TabPageWidget.hpp"
#include "FinTechUI/Model/XTableModel.hpp"
#include "FinTechUI/Model/XSortFilterProxyModel.hpp"
#include "HPPackClient.h"

static QColor GetUserColor(const Message::TLoginResponse& LoginResponse)
{
    QColor color;
    if(QString(LoginResponse.Role).contains("Admin"))
    {
        // 橙红色 #FF4500
        color = QColor("#FF4500");
    }
    else if(QString(LoginResponse.Role).contains("Trader"))
    {
        // 橙色 #FFA500
        color = QColor("#FFA500");
    }
    else
    {   // 深蓝绿色 #00CED1
        color = QColor("#00CED1");
    }
    return color;
}

class PermissionWidget : public FinTechUI::TabPageWidget
{
    Q_OBJECT
public:
    explicit PermissionWidget(QWidget *parent = NULL);
    void UpdateUserPermissionTable(const Message::PackMessage& msg);
protected slots:
    void OnUpdateUserPermission(const QModelIndex &index);
private:
    QSplitter* m_Splitter;
    QWidget* m_ControlPannelWidget;
    QTableView* m_PermissionTableView;
    FinTechUI::XTableModel* m_PermissionTableModel;
    FinTechUI::XSortFilterProxyModel* m_PermissionProxyModel;
    QMap<QString, FinTechUI::XTableModelRow*> m_PermissionMap;
};

#endif // PERMISSIONWIDGET_H
