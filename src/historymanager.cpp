#include "historymanager.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QCoreApplication>
#include <QDir>
#include <QDateTime>

HistoryManager::HistoryManager(QObject *parent)
    : QObject(parent)
{
}

bool HistoryManager::initialize()
{
    m_database = QSqlDatabase::addDatabase("QSQLITE");

    QString path = QCoreApplication::applicationDirPath() + "/browser.db";

    qDebug() << "Database path:" << path;

    m_database.setDatabaseName(path);

    if (!m_database.open())
    {
        qDebug() << "Failed:"
                 << m_database.lastError().text();
        return false;
    }

    QSqlQuery query;

    if (!query.exec(
            "CREATE TABLE IF NOT EXISTS History ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "url TEXT NOT NULL,"
            "title TEXT NOT NULL,"
            "visited_at TEXT NOT NULL)"
            ))
    {
        qDebug() << "Failed to create History table:"
                 << query.lastError().text();
        return false;
    }
    qDebug() << "History table ready";
    return true;
}

void HistoryManager::addHistory(const QUrl &url,const QString &title)
{
    QSqlQuery query;

    query.prepare(
        "INSERT INTO HISTORY "
        "(url, title, visited_at) "
        "VALUES (?, ?, ?)"
        );

    query.addBindValue(url.toString());
    query.addBindValue(title);
    query.addBindValue(QDateTime::currentDateTime().toString(Qt::ISODate));

    if(!query.exec()){
        qDebug()<< "Failed to insert history"
                 <<query.lastError().text();
    }

}

QVector<HistoryEntry> HistoryManager::allHistory()
{
    QSqlQuery query;

    query.prepare(
        "SELECT id, url, title, visited_at "
        "FROM History "
        "ORDER BY visited_at DESC"
        );

    if(!query.exec()){
        qDebug()<< "Failed to retrieve"
                 <<query.lastError().text();

        return {};
    }

    QVector<HistoryEntry> history;

    while(query.next())
    {
        HistoryEntry entry;

        entry.id=query.value(0).toInt();
        entry.url=query.value(1).toString();
        entry.title=query.value(2).toString();
        entry.visitedAt=query.value(3).toString();

        history.push_back(entry);
    }

    return history;
}

QVector<HistoryEntry> HistoryManager::searchHistory(const QString &text)
{
    QSqlQuery query;

    query.prepare(
        "SELECT * "
        "FROM History "
        "WHERE title LIKE ? "
        "OR url LIKE ? "
        "ORDER BY visited_at DESC "
        );

    query.addBindValue("%" + text + "%");
    query.addBindValue("%" + text + "%");

    if(!query.exec()){
        qDebug()<< "Failed to fetch"
                 <<query.lastError().text();
        return {};
    }

    QVector<HistoryEntry> history;

    while(query.next())
    {
        HistoryEntry entry;

        entry.id = query.value("id").toInt();
        entry.url = query.value("url").toString();
        entry.title = query.value("title").toString();
        entry.visitedAt = query.value("visited_at").toString();

        history.push_back(entry);
    }

    return history;
}

void HistoryManager::clearHistory()
{
    QSqlQuery query;

    query.prepare(
        "DELETE FROM History "
        );

    if(!query.exec()){
        qDebug() <<"Failed to delete"
                 <<query.lastError().text();
    }
}

