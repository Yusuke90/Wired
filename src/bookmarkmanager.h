#pragma once

#include <QObject>
#include <QSqlDatabase>
#include <QUrl>

struct BookMarkEntry
{
    int id;
    QString url;
    QString title;
};

class BookMarkManager : public QObject
{
    Q_OBJECT

public:

    explicit BookMarkManager(QObject *parent = nullptr);

    bool initialize();

    void addBookMark(const QUrl &url,
                    const QString &title);

    void removeBookMark(const QString &text);

    QVector<BookMarkEntry> allBookMark();

    bool isBookMarked(const QUrl &url);

private:
    QSqlDatabase m_database;

};