#pragma once

#include <QMainWindow>
#include <QToolBar>
#include <QLineEdit>
#include <QProgressBar>
#include <QStatusBar>
#include <QLabel>
#include <QTabWidget>
#include <QWebEngineView>
#include <QPushButton>
#include <QWebEngineProfile>
#include "downloadmanager.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override = default;

public slots:
    void addNewTab(const QUrl &url = QUrl("https://www.google.com"));
    void addNewIncognitoTab(const QUrl &url = QUrl("https://www.google.com"));

private slots:
    void onAddressEntered();
    void onTabCloseRequested(int index);
    void onPopupBlocked(const QUrl &url);

private:
    void createTab(QWebEngineProfile *profile,
                   const QUrl &url,
                   const QString &prefix = "");
    QWebEngineView* currentWebView();
    void setupToolBar();
    void setupMenuBar();
    void setupStatusBar();

    QTabWidget      *m_tabWidget        = nullptr;
    QToolBar        *m_toolBar          = nullptr;
    QLineEdit       *m_addressBar       = nullptr;
    QAction         *m_backAction       = nullptr;
    QAction         *m_forwardAction    = nullptr;
    QAction         *m_reloadAction     = nullptr;
    QAction         *m_homeAction       = nullptr;
    QProgressBar    *m_progressBar      = nullptr;
    QLabel          *m_progressLabel    = nullptr;
    QWidget         *m_notificationBar  = nullptr;
    DownloadManager *m_downloadManager  = nullptr;
};