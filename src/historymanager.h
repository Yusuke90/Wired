#pragma once

#include <QObject>
#include <QSqlDatabase>
#include <QUrl>

struct HistoryEntry
{
    int id;
    QString url;
    QString title;
    QString visitedAt;
};

class HistoryManager : public QObject
{
    Q_OBJECT

public:
    explicit HistoryManager(QObject *parent = nullptr);

    bool initialize();

    void addHistory(const QUrl &url,
                    const QString &title);

    void clearHistory();

    QVector<HistoryEntry> allHistory();

    QVector<HistoryEntry> searchHistory();

private:
    QSqlDatabase m_database;
};