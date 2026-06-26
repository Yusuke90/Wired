#include "bookmarkdialog.h"

BookmarkDialog::BookmarkDialog(BookMarkManager *bookmarks, QWidget *parent)
    : QDialog(parent), m_bookmarks(bookmarks)
{
    setWindowTitle(tr("Bookmarks"));
    setMinimumSize(600, 400);

    setStyleSheet(
        "QDialog { background: #1a1a2e; color: white; }"
        "QListWidget {"
        "  background: #16213e; color: white;"
        "  border: 1px solid #0f3460; border-radius: 8px;"
        "}"
        "QListWidget::item:hover { background: #0f3460; }"
        "QListWidget::item:selected { background: #e94560; }"
        "QPushButton {"
        "  background: #e94560; color: white;"
        "  border-radius: 6px; padding: 6px 16px;"
        "}"
        "QPushButton:hover { background: #c73652; }"
        );

    QVBoxLayout *layout = new QVBoxLayout(this);

    m_listWidget = new QListWidget(this);
    layout->addWidget(m_listWidget);

    QPushButton *removeBtn = new QPushButton(tr("Remove Selected"), this);
    layout->addWidget(removeBtn);

    populate();

    connect(m_listWidget, &QListWidget::itemDoubleClicked,
            this, &BookmarkDialog::onItemDoubleClicked);

    connect(removeBtn, &QPushButton::clicked,
            this, &BookmarkDialog::onRemove);
}

void BookmarkDialog::populate()
{
    m_listWidget->clear();
    const auto entries = m_bookmarks->allBookMark();
    for (const auto &entry : entries) {
        QListWidgetItem *item = new QListWidgetItem(
            entry.title.isEmpty() ? entry.url : entry.title + "\n" + entry.url
            );
        item->setData(Qt::UserRole, entry.url);
        m_listWidget->addItem(item);
    }
}

void BookmarkDialog::onItemDoubleClicked(QListWidgetItem *item)
{
    QUrl url(item->data(Qt::UserRole).toString());
    emit urlSelected(url);
    accept();
}

void BookmarkDialog::onRemove()
{
    QListWidgetItem *item = m_listWidget->currentItem();
    if (!item) return;

    QString url = item->data(Qt::UserRole).toString();
    m_bookmarks->removeBookMark(url);
    populate();
}