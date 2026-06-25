#include "mainwindow.h"
#include <QWebEngineView>
#include <QAction>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMenuBar>
#include <QProgressBar>
#include <QStyle>
#include <QToolBar>
#include <QStatusBar>
#include <QShortcut>
#include <QKeySequence>
#include <QToolButton>
#include "custombrowserpage.h"
#include <QWebEngineProfile>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QMessageBox>
#include <QWebEnginePermission>
#include <QWebEngineCertificateError>
#include <QWebEngineProfile>
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle(tr("Qt Browser"));
        setupToolBar();
    setupMenuBar();
    setupStatusBar();

    // Module 4 — Tab widget replaces single web view
    m_tabWidget = new QTabWidget(this);

    // ---------- Styling ----------

    m_tabWidget->setStyleSheet(
        "QTabWidget::pane { border: none; }"

        "QTabBar::tab {"
        "background:#16213e;"
        "color:#aaa;"
        "padding:6px 16px;"
        "border-top-left-radius:6px;"
        "border-top-right-radius:6px;"
        "margin-right:2px;"
        "}"

        "QTabBar::tab:selected {"
        "background:#0f3460;"
        "color:white;"
        "}"

        "QTabBar::tab:hover {"
        "background:#0f3460;"
        "}"
        );

    statusBar()->setStyleSheet(
        "QStatusBar {"
        "background:#1a1a2e;"
        "color:#aaa;"
        "border-top:1px solid #0f3460;"
        "}"
        );

    setStyleSheet(
        "QMainWindow {"
        "background:#1a1a2e;"
        "}"
        );

    // ---------- End Styling ----------

    m_tabWidget->setTabsClosable(true);
    m_tabWidget->setMovable(true);

    setCentralWidget(m_tabWidget);

    connect(
        m_tabWidget,
        &QTabWidget::tabCloseRequested,
        this,
        &MainWindow::onTabCloseRequested
        );

    QToolButton *newTabButton =
        new QToolButton(this);

    newTabButton->setText("+");

    connect(
        newTabButton,
        &QToolButton::clicked,
        this,
        [this]()
        {
            addNewTab();
        }
        );

    m_tabWidget->setCornerWidget(
        newTabButton
        );

    connect(
        m_tabWidget,
        &QTabWidget::currentChanged,
        this,
        [this](int)
        {
            QWebEngineView *view =
                currentWebView();

            if (!view)
                return;

            m_addressBar->setText(
                view->url().toString()
                );

            setWindowTitle(
                view->title()
                );

            statusBar()->showMessage(
                "Ready"
                );
        }
        );

    // Wire address bar
    connect(
        m_addressBar,
        &QLineEdit::returnPressed,
        this,
        &MainWindow::onAddressEntered
        );

    // Open first tab
    addNewTab(
        QUrl(
            "https://www.google.com"
            )
        );

    // Download manager
    m_downloadManager =
        new DownloadManager(
            this
            );

    connect(
        QWebEngineProfile::defaultProfile(),
        &QWebEngineProfile::downloadRequested,
        this,
        [this]
        (
            QWebEngineDownloadRequest
            *download
            )
        {
            m_downloadManager
                ->addDownload(
                    download
                    );
        }
        );
}


void MainWindow::createTab(QWebEngineProfile *profile,
                           const QUrl &url,
                           const QString &prefix){
    QWebEngineView *webView = new QWebEngineView(this);

    CustomBrowserPage *page =
        new CustomBrowserPage(profile, webView);

    connect(
        page,
        &QWebEnginePage::certificateError,
        this,
        [this](QWebEngineCertificateError error)
        {
            auto reply = QMessageBox::warning(
                this,
                "HTTPS Certificate Warning",
                QString(
                    "The certificate for\n\n%1\n\nis invalid.\n\nContinue anyway?"
                    ).arg(error.url().toString()),
                QMessageBox::Yes | QMessageBox::No
                );

            if (reply == QMessageBox::Yes)
                error.acceptCertificate();
            else
                error.rejectCertificate();
        }
        );

    connect(
        page,
        &QWebEnginePage::permissionRequested,
        this,
        [this](QWebEnginePermission permission)
        {
            QString permissionName;

            switch (permission.permissionType())
            {
            case QWebEnginePermission::PermissionType::Geolocation:
                permissionName = "Location";
                break;

            case QWebEnginePermission::PermissionType::MediaAudioCapture:
                permissionName = "Microphone";
                break;

            case QWebEnginePermission::PermissionType::MediaVideoCapture:
                permissionName = "Camera";
                break;

            case QWebEnginePermission::PermissionType::Notifications:
                permissionName = "Notifications";
                break;

            default:
                permissionName = "Permission";
                break;
            }

            auto reply = QMessageBox::question(
                this,
                "Permission Request",
                QString("%1 wants access to your %2.\nAllow?")
                    .arg(permission.origin().host())
                    .arg(permissionName)
                );

            if (reply == QMessageBox::Yes)
                permission.grant();
            else
                permission.deny();
        }
        );
    webView->setPage(page);

    int index = m_tabWidget->addTab(webView, prefix +tr("New Tab"));
    m_tabWidget->setCurrentIndex(index);
    // Update tab title when page title changes
    connect(webView, &QWebEngineView::titleChanged,
            this, [this, webView,prefix](const QString &title) {
                int i = m_tabWidget->indexOf(webView);
                if (i >= 0)
                    m_tabWidget->setTabText(i, prefix + (title.isEmpty() ? tr("New Tab") : title));
                if (m_tabWidget->currentWidget() == webView)
                    setWindowTitle(title);
            });

    // Update address bar when URL changes
    connect(webView, &QWebEngineView::urlChanged,
            this, [this, webView](const QUrl &newUrl) {
                if (m_tabWidget->currentWidget() == webView)
                    m_addressBar->setText(newUrl.toString());
            });

    //update favicon
    connect(webView,&QWebEngineView::iconChanged,this,
            [this,webView](const QIcon &icon){
                int i=m_tabWidget->indexOf(webView);
                if(i>=0){
                    m_tabWidget->setTabIcon(i, icon);
                }
            }
            );

    // Update progress bar
    connect(webView, &QWebEngineView::loadProgress,
            this, [this, webView](int progress) {
                if (m_tabWidget->currentWidget() == webView) {
                    m_progressBar->setValue(progress);
                    m_progressLabel->setText(QString::number(progress) + "%");
                }
            });

    // Update status bar
    connect(webView, &QWebEngineView::loadStarted,
            this, [this, webView]() {
                if (m_tabWidget->currentWidget() == webView)
                    statusBar()->showMessage(tr("Loading..."));
            });

    connect(webView, &QWebEngineView::loadFinished,
            this, [this, webView](bool) {
                if (m_tabWidget->currentWidget() == webView)
                    statusBar()->showMessage(tr("Ready"));
            });

    connect(page, &CustomBrowserPage::popupBlocked,
            this, &MainWindow::onPopupBlocked);

    webView->setUrl(url);
}

void MainWindow::addNewTab(const QUrl &url)
{
    createTab(
        QWebEngineProfile::defaultProfile(),
        url
        );
}

void MainWindow::addNewIncognitoTab(const QUrl &url){
    createTab(
        new QWebEngineProfile(this),
        url,
        "🕶 "
        );
}

void MainWindow::onTabCloseRequested(int index)
{
    // Always keep at least one tab open
    if (m_tabWidget->count() <= 1)
        return;

    QWidget *tab = m_tabWidget->widget(index);
    m_tabWidget->removeTab(index);
    delete tab;
}

void MainWindow::onAddressEntered()
{
    QWebEngineView *webView = currentWebView();
    if (webView)
        webView->setUrl(QUrl::fromUserInput(m_addressBar->text()));
}

void MainWindow::setupToolBar()
{
    m_toolBar = addToolBar(tr("Navigation"));
    m_toolBar->setMovable(false);
    m_toolBar->setFloatable(false);

    // Force toolbar to sit below menu bar by setting tool button style
    m_toolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);

    m_backAction    = m_toolBar->addAction(style()->standardIcon(QStyle::SP_ArrowBack),    tr("Back"));
    m_forwardAction = m_toolBar->addAction(style()->standardIcon(QStyle::SP_ArrowForward), tr("Forward"));
    m_reloadAction  = m_toolBar->addAction(style()->standardIcon(QStyle::SP_BrowserReload),tr("Reload"));
    m_homeAction    = m_toolBar->addAction(style()->standardIcon(QStyle::SP_DirHomeIcon),  tr("Home"));

    m_addressBar = new QLineEdit(this);
    m_addressBar->setPlaceholderText(tr("Search or enter address"));
    m_addressBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_addressBar->setMinimumWidth(350);
    m_addressBar->setStyleSheet(
        "QLineEdit {"
        "  background: #16213e;"
        "  color: white;"
        "  border: 1px solid #0f3460;"
        "  border-radius: 12px;"
        "  padding: 4px 12px;"
        "}"
        "QLineEdit:focus {"
        "  border-color: #e94560;"
        "}"
        );
    m_toolBar->addWidget(m_addressBar);

    m_toolBar->setStyleSheet(
        "QToolBar {"
        "  background: #1a1a2e;"
        "  border-bottom: 1px solid #0f3460;"
        "  padding: 4px;"
        "  spacing: 4px;"
        "}"
        "QToolButton {"
        "  color: white;"
        "  background: transparent;"
        "  border-radius: 6px;"
        "  padding: 4px;"
        "}"
        "QToolButton:hover {"
        "  background: #16213e;"
        "}"
        );

    connect(m_backAction,    &QAction::triggered, this, [this]() { if (auto *v = currentWebView()) v->back(); });
    connect(m_forwardAction, &QAction::triggered, this, [this]() { if (auto *v = currentWebView()) v->forward(); });
    connect(m_reloadAction,  &QAction::triggered, this, [this]() { if (auto *v = currentWebView()) v->reload(); });
}

void MainWindow::setupMenuBar()
{
    menuBar()->setStyleSheet(
        "QMenuBar {"
        "  background: #1a1a2e;"
        "  color: white;"
        "  border-bottom: 1px solid #0f3460;"
        "}"
        "QMenuBar::item {"
        "  background: transparent;"
        "  padding: 6px 14px;"
        "}"
        "QMenuBar::item:selected {"
        "  background: #16213e;"
        "  border-radius: 4px;"
        "}"
        "QMenu {"
        "  background: #16213e;"
        "  color: white;"
        "  border: 1px solid #0f3460;"
        "}"
        "QMenu::item:selected {"
        "  background: #0f3460;"
        "}"
        );

    QMenu *fileMenu = menuBar()->addMenu(tr("File"));
    QAction *newTabAction = fileMenu->addAction(tr("New Tab"));
    newTabAction->setShortcut(QKeySequence("Ctrl+T"));
    connect(newTabAction, &QAction::triggered, this, [this]() { addNewTab(); });

    QAction *incognitoAction = fileMenu->addAction(tr("New Incognito Tab"));
    incognitoAction->setShortcut(QKeySequence("Ctrl+Shift+N"));
    connect(incognitoAction, &QAction::triggered, this, [this]() { addNewIncognitoTab(); });

    QAction *closeTabAction = fileMenu->addAction(tr("Close Tab"));
    closeTabAction->setShortcut(QKeySequence("Ctrl+W"));
    connect(closeTabAction, &QAction::triggered, this, [this]() { onTabCloseRequested(m_tabWidget->currentIndex()); });

    fileMenu->addSeparator();
    QAction *quitAction = fileMenu->addAction(tr("Quit"));
    quitAction->setShortcut(QKeySequence("Ctrl+Q"));
    connect(quitAction, &QAction::triggered, this, &QWidget::close);

    QMenu *editMenu = menuBar()->addMenu(tr("Edit"));
    editMenu->addAction(tr("Cut"));
    editMenu->addAction(tr("Copy"));
    editMenu->addAction(tr("Paste"));

    QMenu *viewMenu = menuBar()->addMenu(tr("View"));
    viewMenu->addAction(tr("Zoom In"));
    viewMenu->addAction(tr("Zoom Out"));
    viewMenu->addAction(tr("Reset Zoom"));
    viewMenu->addAction(tr("Toggle Full Screen"));

    QMenu *bookmarksMenu = menuBar()->addMenu(tr("Bookmarks"));
    bookmarksMenu->addAction(tr("Add Bookmark"));
    bookmarksMenu->addAction(tr("Show All Bookmarks"));

    QMenu *historyMenu = menuBar()->addMenu(tr("History"));
    historyMenu->addAction(tr("Show History"));
    historyMenu->addAction(tr("Clear History"));

    QMenu *helpMenu = menuBar()->addMenu(tr("Help"));
    helpMenu->addAction(tr("About"));
}

QWebEngineView* MainWindow::currentWebView()
{
    return qobject_cast<QWebEngineView*>(
        m_tabWidget->currentWidget()
        );
}
void MainWindow::setupStatusBar()
{
    statusBar()->showMessage(tr("Ready"));

    m_progressBar = new QProgressBar(this);
    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(0);
    m_progressBar->setTextVisible(false);
    m_progressBar->setFixedSize(150, 14);
    statusBar()->addPermanentWidget(m_progressBar);

    m_progressLabel = new QLabel(tr("0%"), this);
    m_progressLabel->setFixedWidth(32);
    statusBar()->addPermanentWidget(m_progressLabel);
}
void MainWindow::onPopupBlocked(const QUrl &url)
{
    if (m_notificationBar) {
        m_notificationBar->deleteLater();
        m_notificationBar = nullptr;
    }

    m_notificationBar = new QWidget(this);
    m_notificationBar->setStyleSheet(
        "background-color: #2b2b2b;"
        "border-bottom: 1px solid #555;"
        "color: white;"
        );
    m_notificationBar->setFixedHeight(36);

    QHBoxLayout *layout = new QHBoxLayout(m_notificationBar);
    layout->setContentsMargins(8, 4, 8, 4);

    QLabel *label = new QLabel(tr("Popup blocked"), m_notificationBar);
    QPushButton *openBtn = new QPushButton(tr("Open anyway"), m_notificationBar);
    QPushButton *dismissBtn = new QPushButton(tr("Dismiss"), m_notificationBar);

    layout->addWidget(label);
    layout->addStretch();
    layout->addWidget(openBtn);
    layout->addWidget(dismissBtn);
    openBtn->setStyleSheet("background-color: #0078d4; color: white; border-radius: 4px; padding: 4px 10px;");
    dismissBtn->setStyleSheet("background-color: #555; color: white; border-radius: 4px; padding: 4px 10px;");
    connect(openBtn, &QPushButton::clicked, this, [this, url]() {
        addNewTab(url);
        if (m_notificationBar) {
            m_notificationBar->deleteLater();
            m_notificationBar = nullptr;
        }
    });

    connect(dismissBtn, &QPushButton::clicked, this, [this]() {
        if (m_notificationBar) {
            m_notificationBar->deleteLater();
            m_notificationBar = nullptr;
        }
    });

    // Position bar just below the toolbar
    int toolbarBottom = m_toolBar->y() + m_toolBar->height();
    m_notificationBar->setParent(this);
    m_notificationBar->setGeometry(0, toolbarBottom, this->width(), 36);
    m_notificationBar->show();
    m_notificationBar->raise();
}