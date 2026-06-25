#pragma once

#include <QDialog>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QProgressBar>
#include <QScrollArea>
#include <QWebEngineDownloadRequest>

// ── DownloadItem ──────────────────────────────────────────────────────────────

class DownloadItem : public QWidget
{
    Q_OBJECT

public:
    explicit DownloadItem(QWebEngineDownloadRequest *download, QWidget *parent = nullptr);

private slots:
    void onPauseResume();
    void onCancel();
    void onOpenFolder();

private:
    QWebEngineDownloadRequest *m_download    = nullptr;
    QLabel       *m_fileLabel               = nullptr;
    QLabel       *m_sizeLabel               = nullptr;
    QProgressBar *m_progressBar             = nullptr;
    QPushButton  *m_pauseBtn                = nullptr;
    QPushButton  *m_cancelBtn               = nullptr;
    QPushButton  *m_folderBtn               = nullptr;
};

// ── DownloadManager ───────────────────────────────────────────────────────────

class DownloadManager : public QDialog
{
    Q_OBJECT

public:
    explicit DownloadManager(QWidget *parent = nullptr);
    void addDownload(QWebEngineDownloadRequest *download);

private:
    QVBoxLayout *m_itemsLayout = nullptr;
    int          m_count       = 0;
};