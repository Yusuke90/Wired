#include "bookmarkmanager.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QCoreApplication>
#include <QDir>

BookMarkManager::BookMarkManager(QObject *parent)
    : QObject(parent)
{
}

bool BookMarkManager::initialize()
{
    if(QSqlDatabase::contains())
    {
        m_database=QSqlDatabase::database();
    }
    else
    {
        m_database=QSqlDatabase::addDatabase("QSQLITE");
        m_database.setDatabaseName("browser.db");
        if (!m_database.open())
        {
            qDebug() << "Failed to open database:"
                     << m_database.lastError().text();
            return false;
        }
    }
    QSqlQuery query;

    if (!query.exec(
            "CREATE TABLE IF NOT EXISTS Bookmarks ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "url TEXT NOT NULL,"
            "title TEXT NOT NULL) "
            ))
    {
        qDebug() << "Failed to create Bookmarks table:"
                 << query.lastError().text();
        return false;
    }
    qDebug() << "Bookmarks table ready";
    return true;
}

void BookMarkManager::addBookMark(const QUrl &url,const QString &title)
{
    QSqlQuery query;

    query.prepare(
        " INSERT INTO Bookmarks"
        " (url,title) "
        " VALUES (?,?) "
        );

    query.addBindValue(url.toString());
    query.addBindValue(title);

    if(!query.exec()){
        qDebug()<< "Failed to insert bookmark"
                 <<query.lastError().text();
    }
}

QVector<BookMarkEntry> BookMarkManager::allBookMark()
{
    QSqlQuery query;

    query.prepare(
        " SELECT * "
        " FROM Bookmarks "
        );

    if(!query.exec()){
        qDebug() <<"Failed to fetch"
                 <<query.lastError().text();
        return {};
    }

    QVector<BookMarkEntry> bookmarks;

    while(query.next())
    {
        BookMarkEntry entry;
        entry.id=query.value(0).toInt();
        entry.url=query.value(1).toString();
        entry.title=query.value(2).toString();

        bookmarks.push_back(entry);
    }

    return bookmarks;
}

void BookMarkManager::removeBookMark(const QString &title)
{
    QSqlQuery query;

    query.prepare(
        "DELETE FROM Bookmarks "
        "WHERE title = ? "
        );

    query.addBindValue(title);

    if(!query.exec())
    {
        qDebug()<<"Failed to delete"
                 <<query.lastError().text();
    }
}

bool BookMarkManager::isBookMarked(const QUrl &url)
{
    QSqlQuery query;

    query.prepare(
          " SELECT COUNT(*) "
          " FROM Bookmarks "
          " WHERE url = ? "
        );

    query.addBindValue(url.toString());

    if (!query.exec())
    {
        qDebug() << "Failed to check bookmark:"
                 << query.lastError().text();
        return false;
    }

    query.next();   // Move to the first (and only) row

    int count = query.value(0).toInt();

    return count > 0;
}