#pragma once

#include <QWidget>
#include <QListWidget>
#include <QListWidgetItem>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include "bookmarkmanager.h"

class BookmarkTabWidget : public QWidget
{
    Q_OBJECT

public:
    explicit BookmarkTabWidget(BookMarkManager *bookmarks, QWidget *parent = nullptr);

signals:
    void urlSelected(const QUrl &url);

private slots:
    void onItemDoubleClicked(QListWidgetItem *item);
    void onRemove();
    void onSearch(const QString &text);

private:
    void populate(const QString &filter = QString());
    void setupUI();

    BookMarkManager *m_bookmarks  = nullptr;
    QLineEdit       *m_searchBar  = nullptr;
    QListWidget     *m_listWidget = nullptr;
    QLabel          *m_countLabel = nullptr;
};
