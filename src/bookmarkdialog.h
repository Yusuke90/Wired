#pragma once

#include <QDialog>
#include <QListWidget>
#include <QListWidgetItem>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include "bookmarkmanager.h"

class BookmarkDialog : public QDialog
{
    Q_OBJECT

public:
    explicit BookmarkDialog(BookMarkManager *bookmarks, QWidget *parent = nullptr);

signals:
    void urlSelected(const QUrl &url);

private slots:
    void onItemDoubleClicked(QListWidgetItem *item);
    void onRemove();

private:
    void populate();

    BookMarkManager *m_bookmarks  = nullptr;
    QListWidget     *m_listWidget = nullptr;
};