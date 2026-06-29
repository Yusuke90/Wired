#include "bookmarktabwidget.h"

BookmarkTabWidget::BookmarkTabWidget(BookMarkManager *bookmarks, QWidget *parent)
    : QWidget(parent), m_bookmarks(bookmarks)
{
    setupUI();
    populate();

    connect(m_listWidget, &QListWidget::itemDoubleClicked,
            this, &BookmarkTabWidget::onItemDoubleClicked);
    connect(m_searchBar, &QLineEdit::textChanged,
            this, &BookmarkTabWidget::onSearch);
}

void BookmarkTabWidget::setupUI()
{
    setStyleSheet(
        "QWidget { background: #292a2d; color: #e8eaed; font-family: 'Segoe UI', Roboto, sans-serif; }"
        "QLineEdit {"
        "  background: #303134; color: #e8eaed; border: 1px solid #5f6368;"
        "  border-radius: 4px; padding: 8px 14px; font-size: 13px;"
        "}"
        "QLineEdit:focus { border-color: #8ab4f8; }"
        "QListWidget {"
        "  background: #303134; color: #e8eaed; border: 1px solid #5f6368;"
        "  border-radius: 4px; font-size: 12px; outline: none;"
        "}"
        "QListWidget::item { padding: 10px 14px; border-bottom: 1px solid #3c4043; }"
        "QListWidget::item:hover { background: #3c4043; }"
        "QListWidget::item:selected { background: #8ab4f8; color: #202124; }"
        "QPushButton {"
        "  background: transparent; color: #e8eaed; border: 1px solid #5f6368;"
        "  border-radius: 4px; padding: 8px 20px; font-size: 12px;"
        "}"
        "QPushButton:hover { background: #3c4043; }"
        "QLabel { color: #9aa0a6; font-size: 11px; border: none; }"
        "QScrollBar:vertical { background: #303134; width: 8px; border-radius: 4px; }"
        "QScrollBar::handle:vertical { background: #5f6368; border-radius: 4px; min-height: 20px; }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }"
    );

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 24, 24, 24);
    layout->setSpacing(16);

    // Header
    QHBoxLayout *headerRow = new QHBoxLayout();
    QLabel *title = new QLabel(tr("Bookmarks"), this);
    title->setStyleSheet("font-size: 20px; font-weight: 400; color: #e8eaed; border: none;");

    m_countLabel = new QLabel(this);
    m_countLabel->setStyleSheet("border: none;");

    headerRow->addWidget(title);
    headerRow->addStretch();
    headerRow->addWidget(m_countLabel);
    layout->addLayout(headerRow);

    // Search
    m_searchBar = new QLineEdit(this);
    m_searchBar->setPlaceholderText(tr("Search bookmarks..."));
    layout->addWidget(m_searchBar);

    // List
    m_listWidget = new QListWidget(this);
    layout->addWidget(m_listWidget, 1);

    // Actions row
    QHBoxLayout *btnRow = new QHBoxLayout();
    QPushButton *removeBtn = new QPushButton(tr("Remove selected"), this);
    btnRow->addStretch();
    btnRow->addWidget(removeBtn);
    layout->addLayout(btnRow);

    connect(removeBtn, &QPushButton::clicked, this, &BookmarkTabWidget::onRemove);
}

void BookmarkTabWidget::populate(const QString &filter)
{
    m_listWidget->clear();
    const auto entries = m_bookmarks->allBookMark();
    int count = 0;
    for (const auto &entry : entries) {
        if (!filter.isEmpty() &&
            !entry.title.contains(filter, Qt::CaseInsensitive) &&
            !entry.url.contains(filter, Qt::CaseInsensitive))
            continue;
        QListWidgetItem *item = new QListWidgetItem(
            entry.title.isEmpty() ? entry.url : entry.title + "\n" + entry.url
        );
        item->setData(Qt::UserRole, entry.url);
        m_listWidget->addItem(item);
        count++;
    }
    m_countLabel->setText(tr("%1 bookmarks").arg(count));
}

void BookmarkTabWidget::onItemDoubleClicked(QListWidgetItem *item)
{
    QUrl url(item->data(Qt::UserRole).toString());
    emit urlSelected(url);
}

void BookmarkTabWidget::onRemove()
{
    QListWidgetItem *item = m_listWidget->currentItem();
    if (!item) return;
    QString url = item->data(Qt::UserRole).toString();
    m_bookmarks->removeBookMark(url);
    populate(m_searchBar->text());
}

void BookmarkTabWidget::onSearch(const QString &text)
{
    populate(text);
}
