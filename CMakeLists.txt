cmake_minimum_required(VERSION 3.21)

project(XMonitor LANGUAGES CXX)

SET(AppVersion 0.9.5)
# ---------------------------------------------------------------------------
# C++ 标准
# ---------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_definitions(-DFMT_HEADER_ONLY)
add_definitions(-DFMTLOG_HEADER_ONLY)
add_definitions(-D_GLIBCXX_USE_CXX11_ABI=0)

# 默认 Debug（与原 .pro 中 CONFIG += debug 一致）
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# ---------------------------------------------------------------------------
# 查找 Qt5（Core / Gui / Widgets / PrintSupport）
# ---------------------------------------------------------------------------
find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets PrintSupport)
message(STATUS "Qt version: ${Qt5_VERSION}")
find_program(QMAKE_EXECUTABLE NAMES qmake6 qmake-qt5 qmake)
if(QMAKE_EXECUTABLE)
    execute_process(
        COMMAND ${QMAKE_EXECUTABLE} -v
        OUTPUT_VARIABLE QMAKE_VERSION_OUTPUT
        ERROR_VARIABLE  QMAKE_VERSION_ERROR
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    message(STATUS "qmake path: ${QMAKE_EXECUTABLE} ${QMAKE_VERSION_OUTPUT}")
else()
    message(WARNING "未找到 qmake")
endif()

# Qt 自动处理 moc / uic / rcc
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# 如果 qcustomplot 等第三方 Qt 控件需要 AUTOMOC，也确保开启
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# ---------------------------------------------------------------------------
# 项目路径变量（对应 .pro 中的 CurrentProjectPath）
# ---------------------------------------------------------------------------
set(CurrentProjectPath ${CMAKE_CURRENT_SOURCE_DIR})

# ---------------------------------------------------------------------------
# 源文件与头文件
# ---------------------------------------------------------------------------
set(SOURCES
    main.cpp
    MainWindow.cpp
    HPPackClient.cpp
    Widget/PermissionWidget.cpp
    Widget/MarketWidget.cpp
    Widget/EventLogWidget.cpp
    Widget/RiskJudgeWidget.cpp
    Widget/MonitorWidget.cpp
    Widget/OrderManagerWidget.cpp
    Widget/FuturePosWidget.cpp
    Widget/StockPosWidget.cpp
    FinTechUI/QCustomPlot/qcustomplot.cpp
)

set(HEADERS
    FinTechUI/DragTabWidget/DragTabBar.hpp
    FinTechUI/DragTabWidget/DragTabWidget.hpp
    FinTechUI/DragTabWidget/TabDialog.hpp
    FinTechUI/DragTabWidget/TabPageWidget.hpp
    FinTechUI/Model/XHeaderView.hpp
    FinTechUI/Model/YHeaderView.hpp
    FinTechUI/Model/XTableModel.hpp
    FinTechUI/Model/XTableModelItem.hpp
    FinTechUI/Model/FrozenTableView.hpp
    FinTechUI/Model/XSortFilterProxyModel.hpp
    FinTechUI/Model/XPersistentItemDelegate.hpp
    FinTechUI/Model/XButtonDelegate.hpp
    FinTechUI/Model/YButtonDelegate.hpp
    FinTechUI/Model/XProgressDelegate.hpp
    FinTechUI/FilterWidget.hpp
    FinTechUI/QCustomPlot/qcustomplot.h
    MainWindow.h
    HPPackClient.h
    Widget/PermissionWidget.h
    Widget/MarketWidget.h
    Widget/EventLogWidget.h
    Widget/RiskJudgeWidget.h
    Widget/MonitorWidget.h
    Widget/OrderManagerWidget.h
    Widget/FuturePosWidget.h
    Widget/StockPosWidget.h
    ${CurrentProjectPath}/../Utils/PackMessage.hpp
    ${CurrentProjectPath}/../XAPI/FMTLogger/include/FMTLogger.hpp
)

# ---------------------------------------------------------------------------
# 可执行目标
# ---------------------------------------------------------------------------
add_executable(${PROJECT_NAME}_${AppVersion} ${SOURCES} ${HEADERS})
message(STATUS "Target name = ${PROJECT_NAME}")   # 打印出来确认
# ---------------------------------------------------------------------------
# 包含目录
# ---------------------------------------------------------------------------
target_include_directories(${PROJECT_NAME}_${AppVersion} PRIVATE
    ${CurrentProjectPath}/../Utils/
    ${CurrentProjectPath}/../XAPI/HP-Socket/5.8.2/include/
    ${CurrentProjectPath}/../XAPI/YAML-CPP/0.9.0/include/
    ${CurrentProjectPath}/../XAPI/FMTLogger/include/
)

# ---------------------------------------------------------------------------
# 链接第三方静态库
# ---------------------------------------------------------------------------
target_link_libraries(${PROJECT_NAME}_${AppVersion} PRIVATE
    ${CurrentProjectPath}/../XAPI/HP-Socket/5.8.2/lib/libhpsocket4c.a
    ${CurrentProjectPath}/../XAPI/YAML-CPP/0.9.0/lib/libyaml-cpp.a
    ${CurrentProjectPath}/../XAPI/FMTLogger/lib/libfmtlog.a
    ${CurrentProjectPath}/../XAPI/FMTLogger/lib/libfmt.a
)

# ---------------------------------------------------------------------------
# 链接 Qt6 模块
# ---------------------------------------------------------------------------
target_link_libraries(${PROJECT_NAME}_${AppVersion} PRIVATE
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::PrintSupport
)

# ---------------------------------------------------------------------------
# 获取 Git 提交 ID 与分支名（替代 .pro 中的 $$system(...)）
# ---------------------------------------------------------------------------
execute_process(
    COMMAND git rev-parse HEAD
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_COMMIT_ID
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
)
execute_process(
    COMMAND git symbolic-ref --short -q HEAD
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_BRANCH_NAME
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
)

if(NOT GIT_COMMIT_ID)
    set(GIT_COMMIT_ID "unknown")
endif()
if(NOT GIT_BRANCH_NAME)
    set(GIT_BRANCH_NAME "unknown")
endif()

add_definitions(-DAPP_COMMIT_ID=\"${GIT_COMMIT_ID}\") 
add_definitions(-DAPP_BRANCH_NAME=\"${GIT_BRANCH_NAME}\")

# ---------------------------------------------------------------------------
# Linux 下额外链接（线程、dl 等，静态库常需要）
# ---------------------------------------------------------------------------
if(UNIX AND NOT APPLE)
    target_link_libraries(${PROJECT_NAME}_${AppVersion} PRIVATE pthread dl)
endif()

# ---------------------------------------------------------------------------
# 输出目录（可选）
# ---------------------------------------------------------------------------
set_target_properties(${PROJECT_NAME}_${AppVersion} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/
)