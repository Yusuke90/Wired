#pragma once

#include <QWidget>
#include <QListWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include "historymanager.h"

class HistoryTabWidget : public QWidget
{
    Q_OBJECT

public:
    explicit HistoryTabWidget(HistoryManager *history, QWidget *parent = nullptr);

signals:
    void urlSelected(const QUrl &url);

private slots:
    void onSearch(const QString &text);
    void onItemDoubleClicked(QListWidgetItem *item);

private:
    void populate(const QVector<HistoryEntry> &entries);
    void setupUI();

    HistoryManager  *m_history    = nullptr;
    QLineEdit       *m_searchBar  = nullptr;
    QListWidget     *m_listWidget = nullptr;
    QLabel          *m_countLabel = nullptr;
};
