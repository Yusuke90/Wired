#include "historytabwidget.h"
#include <QListWidgetItem>

HistoryTabWidget::HistoryTabWidget(HistoryManager *history, QWidget *parent)
    : QWidget(parent), m_history(history)
{
    setupUI();
    populate(m_history->allHistory());

    connect(m_searchBar, &QLineEdit::textChanged,
            this, &HistoryTabWidget::onSearch);
    connect(m_listWidget, &QListWidget::itemDoubleClicked,
            this, &HistoryTabWidget::onItemDoubleClicked);
}

void HistoryTabWidget::setupUI()
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
        "QLabel { color: #9aa0a6; font-size: 11px; }"
        "QScrollBar:vertical { background: #303134; width: 8px; border-radius: 4px; }"
        "QScrollBar::handle:vertical { background: #5f6368; border-radius: 4px; min-height: 20px; }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }"
    );

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 24, 24, 24);
    layout->setSpacing(16);

    // Header
    QHBoxLayout *headerRow = new QHBoxLayout();
    QLabel *title = new QLabel(tr("History"), this);
    title->setStyleSheet("font-size: 20px; font-weight: 400; color: #e8eaed; border: none;");

    m_countLabel = new QLabel(this);
    m_countLabel->setStyleSheet("border: none;");

    headerRow->addWidget(title);
    headerRow->addStretch();
    headerRow->addWidget(m_countLabel);
    layout->addLayout(headerRow);

    // Search bar
    m_searchBar = new QLineEdit(this);
    m_searchBar->setPlaceholderText(tr("Search history..."));
    layout->addWidget(m_searchBar);

    // List
    m_listWidget = new QListWidget(this);
    layout->addWidget(m_listWidget, 1);

    // Clear button
    QHBoxLayout *btnRow = new QHBoxLayout();
    btnRow->addStretch();
    QPushButton *clearBtn = new QPushButton(tr("Clear browsing data"), this);
    btnRow->addWidget(clearBtn);
    layout->addLayout(btnRow);

    connect(clearBtn, &QPushButton::clicked, this, [this]() {
        m_history->clearHistory();
        m_listWidget->clear();
        m_countLabel->setText(tr("0 entries"));
    });
}

void HistoryTabWidget::onSearch(const QString &text)
{
    Q_UNUSED(text)
    auto all = m_history->allHistory();
    QVector<HistoryEntry> filtered;
    for (const auto &e : all) {
        if (e.url.contains(text, Qt::CaseInsensitive) ||
            e.title.contains(text, Qt::CaseInsensitive))
            filtered.append(e);
    }
    populate(filtered);
}

void HistoryTabWidget::populate(const QVector<HistoryEntry> &entries)
{
    m_listWidget->clear();
    for (const auto &entry : entries) {
        QListWidgetItem *item = new QListWidgetItem(
            entry.title.isEmpty() ? entry.url : entry.title + "\n" + entry.url
        );
        item->setData(Qt::UserRole, entry.url);
        m_listWidget->addItem(item);
    }
    m_countLabel->setText(tr("%1 entries").arg(entries.size()));
}

void HistoryTabWidget::onItemDoubleClicked(QListWidgetItem *item)
{
    QUrl url(item->data(Qt::UserRole).toString());
    emit urlSelected(url);
}
