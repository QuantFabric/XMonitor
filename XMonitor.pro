QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

CONFIG += c++11

CurrentProjectPath = $$system(pwd)

# Utils
INCLUDEPATH += "$$CurrentProjectPath/../Utils/"
#  HPSocket
INCLUDEPATH += "$$CurrentProjectPath/../XAPI/HP-Socket/5.8.2/include/"
LIBS += "$$CurrentProjectPath/../XAPI/HP-Socket/5.8.2/lib/libhpsocket4c.a"
# YAML-CPP
INCLUDEPATH += "$$CurrentProjectPath/../XAPI/YAML-CPP/0.6.3/include/"
LIBS += "$$CurrentProjectPath/../XAPI/YAML-CPP/0.6.3/lib/libyaml-cpp.a"
# SPDLog
INCLUDEPATH += "$$CurrentProjectPath/../XAPI/SPDLog/1.8.5/include/"
LIBS += "$$CurrentProjectPath/../XAPI/SPDLog/1.8.5/lib/libspdlog.a"

SOURCES += \
    main.cpp \
    MainWindow.cpp \
    HPPackClient.cpp \
    $$CurrentProjectPath/../Utils/Logger.cpp \
    Widget/PermissionWidget.cpp \
    FinTechUI/QCustomPlot/qcustomplot.cpp \
    Widget/MarketWidget.cpp \
    Widget/EventLogWidget.cpp \

HEADERS += \
    FinTechUI/DragTabWidget/DragTabBar.hpp \
    FinTechUI/DragTabWidget/DragTabWidget.hpp \
    FinTechUI/DragTabWidget/TabDialog.hpp \
    FinTechUI/DragTabWidget/TabPageWidget.hpp \
    FinTechUI/Model/XHeaderView.hpp \
    FinTechUI/Model/YHeaderView.hpp \
    FinTechUI/Model/XTableModel.hpp \
    FinTechUI/Model/XTableModelItem.hpp \
    FinTechUI/Model/FrozenTableView.hpp \
    FinTechUI/Model/XSortFilterProxyModel.hpp \
    FinTechUI/Model/XPersistentItemDelegate.hpp \
    FinTechUI/Model/XButtonDelegate.hpp \
    FinTechUI/Model/YButtonDelegate.hpp \
    FinTechUI/Model/XProgressDelegate.hpp \
    FinTechUI/FilterWidget.hpp \
    FinTechUI/QCustomPlot/qcustomplot.h \
    MainWindow.h \
    PackMessage.hpp \
    HPPackClient.h \
    Logger.h \
    Widget/PermissionWidget.h \
    Widget/MarketWidget.h \
    Widget/EventLogWidget.h \


win32 {
    DEFINES += APP_COMMIT_ID=\\\"'$$system(cmd /c git rev-parse HEAD)'\\\"
    DEFINES += APP_BRANCH_NAME=\\\"'$$system(cmd /c git symbolic-ref --short -q HEAD)'\\\"
}
else {
    DEFINES += APP_COMMIT_ID=\\\"'$(shell git rev-parse HEAD)'\\\"
    DEFINES += APP_BRANCH_NAME=\\\"'$(shell git symbolic-ref --short -q HEAD)'\\\"
}

TEMPLATE = app
TARGET = XMonitor_0.4.0
