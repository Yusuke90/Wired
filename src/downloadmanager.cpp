#include "downloadmanager.h"
#include <QFileInfo>
#include <QDesktopServices>
#include <QUrl>
#include <QDir>
#include <QScrollArea>
#include <QFrame>

// ── Helpers ───────────────────────────────────────────────────────────────────

static QString formatBytes(qint64 bytes)
{
    if (bytes <= 0)    return QStringLiteral("?");
    if (bytes < 1024)  return QString::number(bytes) + " B";
    if (bytes < 1024 * 1024)
        return QString::number(bytes / 1024.0, 'f', 1) + " KB";
    if (bytes < 1024 * 1024 * 1024)
        return QString::number(bytes / (1024.0 * 1024), 'f', 1) + " MB";
    return QString::number(bytes / (1024.0 * 1024 * 1024), 'f', 2) + " GB";
}

// ── Button style helpers ──────────────────────────────────────────────────────

static const char *kBtnBase =
    "QPushButton {"
    "  border: none;"
    "  border-radius: 4px;"
    "  padding: 3px 10px;"
    "  font-size: 11px;"
    "  font-weight: 600;"
    "}"
    "QPushButton:disabled { opacity: 0.35; }";

static QString btnStyle(const QString &bg, const QString &fg = "#fff")
{
    return QString(kBtnBase) +
           "QPushButton { background:" + bg + "; color:" + fg + "; }"
           "QPushButton:hover { background:" + bg + "cc; }";
}

// ── DownloadItem ──────────────────────────────────────────────────────────────

DownloadItem::DownloadItem(QWebEngineDownloadRequest *download, QWidget *parent)
    : QWidget(parent), m_download(download)
{
    // ---- outer card ----
    setObjectName("dlItem");
    setStyleSheet(
        "QWidget#dlItem {"
        "  background: #1e2a45;"
        "  border: 1px solid #2e4070;"
        "  border-radius: 8px;"
        "}"
    );
    setFixedHeight(90);

    QVBoxLayout *main = new QVBoxLayout(this);
    main->setContentsMargins(12, 8, 12, 8);
    main->setSpacing(4);

    // ---- top row: filename + size ----
    QHBoxLayout *topRow = new QHBoxLayout();
    topRow->setContentsMargins(0, 0, 0, 0);

    QString rawName = QFileInfo(download->downloadFileName()).fileName();
    m_fileLabel = new QLabel(rawName, this);
    m_fileLabel->setStyleSheet("color: #e8eaf6; font-size: 12px; font-weight: 600;");
    m_fileLabel->setMaximumWidth(280);
    m_fileLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    m_fileLabel->setToolTip(rawName);

    m_sizeLabel = new QLabel(QStringLiteral("Connecting…"), this);
    m_sizeLabel->setStyleSheet("color: #8899bb; font-size: 11px;");
    m_sizeLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    topRow->addWidget(m_fileLabel, 1);
    topRow->addWidget(m_sizeLabel);

    // ---- progress bar ----
    m_progressBar = new QProgressBar(this);
    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(0);
    m_progressBar->setFixedHeight(6);
    m_progressBar->setTextVisible(false);
    m_progressBar->setStyleSheet(
        "QProgressBar {"
        "  background: #253050;"
        "  border-radius: 3px;"
        "  border: none;"
        "}"
        "QProgressBar::chunk {"
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0,"
        "    stop:0 #4f7cff, stop:1 #a855f7);"
        "  border-radius: 3px;"
        "}"
    );

    // ---- buttons row ----
    QHBoxLayout *btnRow = new QHBoxLayout();
    btnRow->setContentsMargins(0, 0, 0, 0);
    btnRow->setSpacing(6);

    m_pauseBtn  = new QPushButton(tr("Pause"),       this);
    m_cancelBtn = new QPushButton(tr("Cancel"),      this);
    m_folderBtn = new QPushButton(tr("Open Folder"), this);

    m_pauseBtn ->setStyleSheet(btnStyle("#4f7cff"));
    m_cancelBtn->setStyleSheet(btnStyle("#e94560"));
    m_folderBtn->setStyleSheet(btnStyle("#2e7d32"));
    m_folderBtn->setEnabled(false);

    btnRow->addWidget(m_pauseBtn);
    btnRow->addWidget(m_cancelBtn);
    btnRow->addWidget(m_folderBtn);
    btnRow->addStretch();

    main->addLayout(topRow);
    main->addWidget(m_progressBar);
    main->addLayout(btnRow);

    // ---- signals: progress ----
    connect(m_download, &QWebEngineDownloadRequest::receivedBytesChanged,
            this, [this]() {
                qint64 total    = m_download->totalBytes();
                qint64 received = m_download->receivedBytes();

                if (total > 0) {
                    m_progressBar->setValue(int(received * 100 / total));
                    m_sizeLabel->setText(formatBytes(received) + " / " + formatBytes(total));
                } else {
                    m_progressBar->setRange(0, 0); // indeterminate
                    m_sizeLabel->setText(formatBytes(received));
                }
            });

    // ---- signals: state ----
    connect(m_download, &QWebEngineDownloadRequest::stateChanged,
            this, [this](QWebEngineDownloadRequest::DownloadState state) {
                switch (state) {
                case QWebEngineDownloadRequest::DownloadCompleted:
                    m_progressBar->setRange(0, 100);
                    m_progressBar->setValue(100);
                    m_pauseBtn ->setEnabled(false);
                    m_cancelBtn->setEnabled(false);
                    m_folderBtn->setEnabled(true);
                    m_fileLabel->setText(m_fileLabel->text() + tr(" ✓"));
                    m_sizeLabel->setText(tr("Completed"));
                    break;
                case QWebEngineDownloadRequest::DownloadCancelled:
                    m_pauseBtn ->setEnabled(false);
                    m_cancelBtn->setEnabled(false);
                    m_sizeLabel->setText(tr("Cancelled"));
                    break;
                case QWebEngineDownloadRequest::DownloadInterrupted:
                    m_sizeLabel->setText(tr("Interrupted"));
                    break;
                default:
                    break;
                }
            });

    // ---- signals: buttons ----
    connect(m_pauseBtn,  &QPushButton::clicked, this, &DownloadItem::onPauseResume);
    connect(m_cancelBtn, &QPushButton::clicked, this, &DownloadItem::onCancel);
    connect(m_folderBtn, &QPushButton::clicked, this, &DownloadItem::onOpenFolder);

    // Start the download
    m_download->accept();
}

void DownloadItem::onPauseResume()
{
    if (m_download->isPaused()) {
        m_download->resume();
        m_pauseBtn->setText(tr("Pause"));
    } else {
        m_download->pause();
        m_pauseBtn->setText(tr("Resume"));
    }
}

void DownloadItem::onCancel()
{
    m_download->cancel();
}

void DownloadItem::onOpenFolder()
{
    QFileInfo info(m_download->downloadDirectory() + "/" + m_download->downloadFileName());
    QDesktopServices::openUrl(QUrl::fromLocalFile(info.absolutePath()));
}

// ── DownloadManager ───────────────────────────────────────────────────────────

DownloadManager::DownloadManager(QWidget *parent)
    : QDialog(parent, Qt::Tool | Qt::WindowStaysOnTopHint)
{
    setWindowTitle(tr("Downloads"));
    setMinimumWidth(420);
    setMinimumHeight(100);
    setMaximumHeight(520);

    // ---- overall dialog styling ----
    setStyleSheet(
        "QDialog {"
        "  background: #0f1929;"
        "  border: 1px solid #2e4070;"
        "  border-radius: 10px;"
        "}"
        "QScrollArea { border: none; background: transparent; }"
        "QScrollBar:vertical {"
        "  background: #1a2540;"
        "  width: 6px;"
        "  border-radius: 3px;"
        "}"
        "QScrollBar::handle:vertical {"
        "  background: #4f7cff;"
        "  border-radius: 3px;"
        "  min-height: 20px;"
        "}"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }"
    );

    QVBoxLayout *root = new QVBoxLayout(this);
    root->setContentsMargins(12, 12, 12, 12);
    root->setSpacing(8);

    // ---- header ----
    QHBoxLayout *header = new QHBoxLayout();
    QLabel *title = new QLabel(tr("⬇  Downloads"), this);
    title->setStyleSheet("color: #e8eaf6; font-size: 14px; font-weight: 700;");

    QPushButton *clearBtn = new QPushButton(tr("Clear All"), this);
    clearBtn->setStyleSheet(btnStyle("#253050", "#8899bb"));
    connect(clearBtn, &QPushButton::clicked, this, [this]() {
        // Remove all DownloadItem children from the scroll widget
        QList<DownloadItem*> items = m_itemsLayout->parentWidget()->findChildren<DownloadItem*>();
        for (auto *item : items) {
            m_itemsLayout->removeWidget(item);
            item->deleteLater();
        }
        m_count = 0;
        adjustSize();
    });

    header->addWidget(title);
    header->addStretch();
    header->addWidget(clearBtn);

    // ---- scroll area holding items ----
    QScrollArea *scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QWidget *container = new QWidget(scroll);
    container->setStyleSheet("background: transparent;");
    m_itemsLayout = new QVBoxLayout(container);
    m_itemsLayout->setContentsMargins(0, 0, 0, 0);
    m_itemsLayout->setSpacing(6);
    m_itemsLayout->addStretch();

    scroll->setWidget(container);

    root->addLayout(header);
    root->addWidget(scroll, 1);
}

void DownloadManager::addDownload(QWebEngineDownloadRequest *download)
{
    DownloadItem *item = new DownloadItem(download, nullptr);
    // Insert before the trailing stretch
    m_itemsLayout->insertWidget(m_count, item);
    m_count++;

    // Resize dialog to fit, up to the maximum
    int ideal = 12 + 40 + 8 + m_count * (90 + 6) + 12;
    resize(width(), qMin(ideal, 520));

    // Show as a non-blocking floating window
    if (!isVisible()) {
        // Position bottom-right of the parent window
        if (QWidget *p = parentWidget()) {
            QPoint br = p->mapToGlobal(p->rect().bottomRight());
            move(br.x() - width() - 16, br.y() - height() - 48);
        }
        show();
    }
    raise();
    activateWindow();
}