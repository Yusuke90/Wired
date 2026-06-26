#pragma once

#include <QDialog>
#include <QListWidget>
#include <QListWidgetItem>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include "historymanager.h"

class HistoryDialog : public QDialog
{
    Q_OBJECT

public:
    explicit HistoryDialog(HistoryManager *history, QWidget *parent = nullptr);

signals:
    void urlSelected(const QUrl &url);

private slots:
    void onSearch(const QString &text);
    void onItemDoubleClicked(QListWidgetItem *item);

private:
    void populate(const QVector<HistoryEntry> &entries);

    HistoryManager  *m_history    = nullptr;
    QLineEdit       *m_searchBar  = nullptr;
    QListWidget     *m_listWidget = nullptr;
};