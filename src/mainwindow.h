#pragma once

#include <QMainWindow>
#include <QTabBar>
#include <QStackedWidget>
#include <QLineEdit>
#include <QProgressBar>
#include <QStatusBar>
#include <QLabel>
#include <QWebEngineView>
#include <QPushButton>
#include <QToolButton>
#include <QWebEngineProfile>
#include <QAction>
#include <QMenu>
#include "downloadmanager.h"
#include "historymanager.h"
#include "bookmarkmanager.h"
#include "historydialog.h"
#include "bookmarkdialog.h"
#include <QCompleter>
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
    void onCurrentTabChanged(int index);
    void onPopupBlocked(const QUrl &url);

private:
    void createTab(QWebEngineProfile *profile,
                   const QUrl &url,
                   const QString &prefix = "");
    QWebEngineView* currentWebView();
    void setupTabStrip(QWidget *parent);
    void setupNavBar(QWidget *parent);
    void setupStatusBar();
    void buildHamburgerMenu(QMenu *menu);
    void addApiTesterTab();

    // Tab strip
    QTabBar         *m_tabBar           = nullptr;
    QStackedWidget  *m_webStack         = nullptr;

    // Nav bar
    QWidget         *m_navWidget        = nullptr;
    QLineEdit       *m_addressBar       = nullptr;
    QToolButton     *m_backBtn          = nullptr;
    QToolButton     *m_forwardBtn       = nullptr;
    QToolButton     *m_reloadBtn        = nullptr;
    QToolButton     *m_homeBtn          = nullptr;

    // Status / progress
    QProgressBar    *m_progressBar      = nullptr;
    QLabel          *m_progressLabel    = nullptr;

    // Popup notification
    QWidget         *m_notificationBar  = nullptr;

    DownloadManager *m_downloadManager  = nullptr;
    HistoryManager *m_historyManager = nullptr;
    BookMarkManager *m_bookMarkManager  = nullptr;
    QCompleter *m_completer = nullptr;

    QLabel *m_networkStatusLabel = nullptr;
};