#include "historydialog.h"
#include "historymanager.h"
#include <QListWidgetItem>

HistoryDialog::HistoryDialog(HistoryManager *history, QWidget *parent)
    : QDialog(parent), m_history(history)
{
    setWindowTitle(tr("History"));
    setMinimumSize(600, 400);

    setStyleSheet(
        "QDialog { background: #1a1a2e; color: white; }"
        "QLineEdit {"
        "  background: #16213e; color: white;"
        "  border: 1px solid #0f3460; border-radius: 8px; padding: 6px 12px;"
        "}"
        "QListWidget {"
        "  background: #16213e; color: white;"
        "  border: 1px solid #0f3460; border-radius: 8px;"
        "}"
        "QListWidget::item:hover { background: #0f3460; }"
        "QListWidget::item:selected { background: #e94560; }"
        );

    QVBoxLayout *layout = new QVBoxLayout(this);

    m_searchBar = new QLineEdit(this);
    m_searchBar->setPlaceholderText(tr("Search history..."));
    layout->addWidget(m_searchBar);

    m_listWidget = new QListWidget(this);
    layout->addWidget(m_listWidget);

    QPushButton *clearBtn = new QPushButton(tr("Clear History"), this);
    clearBtn->setStyleSheet(
        "QPushButton {"
        "  background: #e94560; color: white;"
        "  border-radius: 6px; padding: 6px 16px;"
        "}"
        "QPushButton:hover { background: #c73652; }"
        );
    layout->addWidget(clearBtn);

    // Load all history on open
    populate(m_history->allHistory());

    connect(m_searchBar, &QLineEdit::textChanged,
            this, &HistoryDialog::onSearch);

    connect(m_listWidget, &QListWidget::itemDoubleClicked,
            this, &HistoryDialog::onItemDoubleClicked);

    connect(clearBtn, &QPushButton::clicked, this, [this]() {
        m_history->clearHistory();
        m_listWidget->clear();
    });
}

void HistoryDialog::onSearch(const QString &text)
{
    Q_UNUSED(text)
    // searchHistory() has no filter yet — show all and filter locally
    auto all = m_history->allHistory();
    QVector<HistoryEntry> filtered;
    for (const auto &e : all) {
        if (e.url.contains(text, Qt::CaseInsensitive) ||
            e.title.contains(text, Qt::CaseInsensitive))
            filtered.append(e);
    }
    populate(filtered);
}

void HistoryDialog::populate(const QVector<HistoryEntry> &entries)
{
    m_listWidget->clear();
    for (const auto &entry : entries) {
        QListWidgetItem *item = new QListWidgetItem(
            entry.title.isEmpty() ? entry.url : entry.title + "\n" + entry.url
            );
        item->setData(Qt::UserRole, entry.url);
        m_listWidget->addItem(item);
    }
}
void HistoryDialog::onItemDoubleClicked(QListWidgetItem *item)
{
    QUrl url(item->data(Qt::UserRole).toString());
    emit urlSelected(url);
    accept();
}