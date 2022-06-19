#ifndef MARKETWIDGET_H
#define MARKETWIDGET_H

#include <QObject>
#include <QWidget>
#include <QTableView>
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
#include <QString>
#include <QStringList>
#include <QMap>
#include <QLineEdit>
#include <QMultiMap>
#include <QDialogButtonBox>
#include <QQueue>
#include <QThread>
#include <QDebug>
#include <QFile>
#include <QMessageBox>
#include <QCoreApplication>
#include "Logger.h"
#include "PackMessage.hpp"
#include "FinTechUI/DragTabWidget/TabPageWidget.hpp"
#include "FinTechUI/Model/XTableModel.hpp"
#include "FinTechUI/Model/XSortFilterProxyModel.hpp"
#include "FinTechUI/FilterWidget.hpp"
#include "FinTechUI/Model/FrozenTableView.hpp"
#include "FinTechUI/QCustomPlot/qcustomplot.h"

class MarketWidget : public FinTechUI::TabPageWidget
{
    Q_OBJECT
public:
    explicit MarketWidget(QWidget* parent = NULL);

protected:
    void InitFilterWidget();
    void InitMarketTableView();
    void InitKCustomPlot();
    void UpdateFutureData(const Message::PackMessage& data);
    void UpdateSpotData(const Message::PackMessage& data);
    void AppendRow(QString Colo, const MarketData::TFutureMarketData& msg);
    void UpdateRow(QString Colo, const MarketData::TFutureMarketData& msg);
protected slots:
    void OnReceivedFutureData(const QList<Message::PackMessage>&);
    void OnReceivedSpotData(const QList<Message::PackMessage>&);
    void OnReceivedStockData(const QList<Message::PackMessage>&);
    void OnFilterTable(const QVector<QStringList>& filter);
    void OnClicked(const QModelIndex&);
protected:
    void timerEvent(QTimerEvent *event);
protected:
    QSplitter* m_Splitter;
    FinTechUI::FilterWidget* m_FilterWidget;
    QTableView* m_MarketTableView;
    FinTechUI::XTableModel* m_MarketTableModel;
    FinTechUI::XSortFilterProxyModel* m_MarketProxyModel;
    QMap<QString, FinTechUI::XTableModelRow*> m_ColoTickerModelRowMap;
    QMap<QString, QStringList> m_ColoTickerSetMap;
    int m_ReplotTimer;
    QMap<QString, QVector<double>> m_ColoTickerTickMap; // Colo + Ticker, Tick
    QMap<QString, QVector<double>> m_ColoTickerPriceMap; // Colo + Ticker, LastPrice
    QMap<QString, MarketData::TFutureMarketData> m_ColoTickerMarketDataMap;
    QCustomPlot* m_KCustomPlot;
    QCPTextElement* m_TextElement;
};

#endif // MARKETWIDGET_H
