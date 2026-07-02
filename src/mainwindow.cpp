#include "mainwindow.h"
#include "custombrowserpage.h"
#include "apitesterwidget.h"
#include "historymanager.h"
#include "bookmarkmanager.h"
#include "bookmarkdialog.h"
#include <QWebEngineView>
#include <QWebEngineProfile>
#include <QWebEnginePermission>
#include <QWebEngineCertificateError>
#include <QAction>
#include <QApplication>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QProgressBar>
#include <QShortcut>
#include <QKeySequence>
#include <QStackedWidget>
#include <QStatusBar>
#include <QStyle>
#include <QTabBar>
#include <QToolButton>
#include <QPushButton>
#include <QSizePolicy>
#include <QFont>
#include <QNetworkInformation>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QCompleter>
#include <QStringListModel>
#include <QWebEngineFindTextResult>
#include <QInputDialog>
#include <QWebEngineFullScreenRequest>
#include <QWebEngineSettings>

// ─────────────────────────────────────────────────────────────────────────────
//  Chrome dark-mode palette (exact values from chrome://settings)
// ─────────────────────────────────────────────────────────────────────────────
static const char *kFrame       = "#202124";   // window frame / tab bar bg
static const char *kToolbar     = "#35363a";   // toolbar + active tab bg
static const char *kSurface     = "#292a2d";   // inactive tab hover
static const char *kOmnibox     = "#202124";   // address bar bg (darker than toolbar)
static const char *kBorder      = "#3c4043";   // subtle dividers
static const char *kText        = "#e8eaed";   // primary text
static const char *kTextDim     = "#9aa0a6";   // secondary / icon text
static const char *kAccent      = "#8ab4f8";   // focus ring / link blue
static const char *kHover       = "#3c4043";   // button hover

// ─────────────────────────────────────────────────────────────────────────────
//  Font — Chrome uses Segoe UI on Windows, Roboto on Linux
// ─────────────────────────────────────────────────────────────────────────────
static const char *kFontFamily  = "Segoe UI, Roboto, Arial, sans-serif";

// ─────────────────────────────────────────────────────────────────────────────
//  Constructor
// ─────────────────────────────────────────────────────────────────────────────
MainWindow::MainWindow(bool incognitoWindow,QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle(tr("Qt Browser"));
    setWindowIcon(QIcon(":/icon.ico"));
    menuBar()->hide();
    statusBar()->hide();
    // Global font
    QFont appFont(kFontFamily, 10);
    appFont.setWeight(QFont::Normal);
    QApplication::setFont(appFont);

    setStyleSheet(
        QString("QMainWindow { background: %1; }").arg(kFrame)
    );

    // Central layout: tab strip → nav bar → web content
    QWidget     *central = new QWidget(this);
    QVBoxLayout *vl      = new QVBoxLayout(central);
    vl->setSpacing(0);
    vl->setContentsMargins(0, 0, 0, 0);

    // Row 1 — Tab strip
    QWidget *tabStrip = new QWidget(central);
    setupTabStrip(tabStrip);
    vl->addWidget(tabStrip);

    // Row 2 — Navigation bar
    m_navWidget = new QWidget(central);
    setupNavBar(m_navWidget);
    vl->addWidget(m_navWidget);

    // Row 3 — Web content stack
    m_webStack = new QStackedWidget(central);
    vl->addWidget(m_webStack);

    setCentralWidget(central);

    m_historyManager= new HistoryManager(this);
    m_historyManager->initialize();
    // Address bar autocomplete from history
    QCompleter *completer = new QCompleter(this);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setFilterMode(Qt::MatchContains);
    m_addressBar->setCompleter(completer);

    // Update completer model when address bar is focused
    connect(m_addressBar, &QLineEdit::textEdited,
            this, [this, completer](const QString &text) {
                if (text.length() < 2) return;
                const auto entries = m_historyManager->searchHistory(text);
                QStringList urls;
                for (const auto &e : entries)
                    urls << e.url;
                completer->setModel(new QStringListModel(urls, completer));
            });

    m_bookMarkManager = new BookMarkManager(this);
    m_bookMarkManager->initialize();

    // Status bar
    setupStatusBar();

    // Keyboard shortcuts
    auto *scNewTab = new QShortcut(QKeySequence("Ctrl+T"),       this);
    auto *scClose  = new QShortcut(QKeySequence("Ctrl+W"),       this);
    auto *scIncog  = new QShortcut(QKeySequence("Ctrl+Shift+N"), this);
    auto *scQuit   = new QShortcut(QKeySequence("Ctrl+Q"),       this);
    auto *scReload = new QShortcut(QKeySequence("F5"),           this);
    auto *scNewWindow = new QShortcut(QKeySequence("Ctrl+N"),    this);
    auto *scNewIncognitoWindow = new QShortcut(QKeySequence("Ctrl+I"),this);
    auto *scFind   = new QShortcut(QKeySequence("Ctrl+F"),        this);

    connect(scNewTab, &QShortcut::activated, this, [this]() { addNewTab(); });
    connect(scClose,  &QShortcut::activated, this, [this]() {
        onTabCloseRequested(m_tabBar->currentIndex());
    });
    connect(scIncog,  &QShortcut::activated, this, [this]() { addNewIncognitoTab(); });
    connect(scQuit,   &QShortcut::activated, this, &QWidget::close);
    connect(scReload, &QShortcut::activated, this, [this]() {
        if (auto *v = currentWebView()) v->reload();
    });
    connect(scNewWindow,
            &QShortcut::activated,
            this,
            [this]()
            {
                addNewWindow();
            });
    connect(scNewIncognitoWindow,
            &QShortcut::activated,
            this,
            [this]()
            {
                addNewIncognitoWindow();
            });

    connect(scFind,&QShortcut::activated,this,
            [this]()
            {
                if(!currentWebView())
                {
                   return;
                }

                QString text = QInputDialog::getText(
                    this,
                    "Find In Page",
                    "Find:"
                    );

                if(!text.isEmpty())
                {
                    currentWebView()->page()->findText(text);
                }

            });

    // Address bar
    connect(m_addressBar, &QLineEdit::returnPressed,
            this, &MainWindow::onAddressEntered);

    // Download manager
    m_downloadManager = new DownloadManager(this);
    connect(QWebEngineProfile::defaultProfile(),
            &QWebEngineProfile::downloadRequested,
            this, [this](QWebEngineDownloadRequest *dl) {
                m_downloadManager->addDownload(dl);
            });

    QWebEngineProfile *profile = QWebEngineProfile::defaultProfile();
    profile->setHttpCacheType(QWebEngineProfile::DiskHttpCache);
    profile->setHttpCacheMaximumSize(100*1024*1024);

    // First tab
    if (incognitoWindow)
    {
        addNewIncognitoTab(
            QUrl("qrc:///resources/newtab_incognito.html")
            );
    }
    else
    {
        addNewTab();
    }

    QNetworkInformation::loadDefaultBackend();

    QNetworkInformation *networkInfo =
        QNetworkInformation::instance();

    if (!networkInfo)
    {
        m_networkStatusLabel->setText("Network: Unknown");
    }
    else
    {
        auto updateStatus = [this, networkInfo]()
        {
            if (networkInfo->reachability() ==
                QNetworkInformation::Reachability::Online)
            {
                m_networkStatusLabel->setText("🟢 Online");
            }
            else
            {
                m_networkStatusLabel->setText("🔴 Offline");
            }
        };

        connect(networkInfo,
                &QNetworkInformation::reachabilityChanged,
                this,
                updateStatus);

        updateStatus();
    }

    m_suspendTimer = new QTimer(this);
    m_suspendTimer->start(300000);

    connect(
        m_suspendTimer,
        &QTimer::timeout,
        this,
        [this]()
        {
            for (int i = 0; i < m_webStack->count(); i++)
            {
                if (i == m_tabBar->currentIndex())
                    continue;

                auto *view =
                    qobject_cast<QWebEngineView*>(
                        m_webStack->widget(i));

                if (!view)
                    continue;

                // Suspend rendering
                view->setUpdatesEnabled(false);
            }
        });

}

// ─────────────────────────────────────────────────────────────────────────────
//  Tab strip — Chrome layout
//  ╭── Tab 1 ──╮╭── Tab 2 ──╮  +
//  The active tab is the same colour as the toolbar below, creating a
//  seamless connection.  Inactive tabs are darker (frame colour).
// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::setupTabStrip(QWidget *strip)
{
    strip->setFixedHeight(34);
    strip->setObjectName("tabStrip");
    strip->setStyleSheet(
        QString(
            "QWidget#tabStrip { background: %1; }"
        ).arg(kFrame)
    );

    QHBoxLayout *hl = new QHBoxLayout(strip);
    hl->setContentsMargins(8, 4, 4, 0);   // small top padding, no bottom
    hl->setSpacing(0);

    // ── Tab bar ──────────────────────────────────────────────────────────────
    m_tabBar = new QTabBar(strip);
    m_tabBar->setTabsClosable(true);
    m_tabBar->setMovable(true);
    m_tabBar->setExpanding(false);
    m_tabBar->setElideMode(Qt::ElideRight);
    m_tabBar->setDocumentMode(true);

    connect(
        m_tabBar,
        &QTabBar::tabCloseRequested,
        this,
        &MainWindow::onTabCloseRequested
        );

    // Chrome-style tabs: rounded top, active tab = toolbar colour
    m_tabBar->setStyleSheet(
        QString(
            "QTabBar { background: transparent; border: none; }"

            "QTabBar::tab {"
            "  background: %1;"            // frame colour = inactive
            "  color: %2;"
            "  padding: 5px 12px;"
            "  min-width: 100px;"
            "  max-width: 220px;"
            "  margin-right: 1px;"
            "  border-top-left-radius: 8px;"
            "  border-top-right-radius: 8px;"
            "  font-family: '%3';"
            "  font-size: 12px;"
            "}"

            "QTabBar::tab:selected {"
            "  background: %4;"            // toolbar colour = active
            "  color: %5;"
            "}"

            "QTabBar::tab:!selected:hover {"
            "  background: %6;"            // surface = hover
            "}"
        ).arg(kFrame, kTextDim, kFontFamily, kToolbar, kText, kSurface)
    );

    connect(m_tabBar, &QTabBar::currentChanged,
            this, &MainWindow::onCurrentTabChanged);

    hl->addWidget(m_tabBar);

    // ── "+" new-tab button ──────────────────────────────────────────────────
    QToolButton *newTabBtn = new QToolButton(strip);
    newTabBtn->setText("+");
    newTabBtn->setToolTip(tr("New tab (Ctrl+T)"));
    newTabBtn->setFixedSize(28, 28);
    newTabBtn->setStyleSheet(
        QString(
            "QToolButton {"
            "  color: %1;"
            "  background: transparent;"
            "  border: none;"
            "  border-radius: 14px;"
            "  font-size: 18px;"
            "  font-weight: 300;"
            "  font-family: '%2';"
            "}"
            "QToolButton:hover { background: %3; }"
        ).arg(kTextDim, kFontFamily, kSurface)
    );
    connect(newTabBtn, &QToolButton::clicked,
            this, [this]() { addNewTab(); });

    hl->addWidget(newTabBtn);
    hl->addStretch();
}

// ─────────────────────────────────────────────────────────────────────────────
//  Navigation bar — Chrome layout
//  [ ← ] [ → ] [ ⟳ ]   [ 🔒  address bar ...                    ]   [ ⋮ ]
//  Background matches the active tab colour, so they blend together.
// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::setupNavBar(QWidget *nav)
{
    nav->setFixedHeight(40);
    nav->setObjectName("navBar");
    nav->setStyleSheet(
        QString(
            "QWidget#navBar {"
            "  background: %1;"
            "  border-bottom: 1px solid %2;"
            "}"
        ).arg(kToolbar, kBorder)
    );

    QHBoxLayout *hl = new QHBoxLayout(nav);
    hl->setContentsMargins(8, 4, 8, 4);
    hl->setSpacing(0);

    // Chrome-style flat icon buttons
    auto makeBtn = [&](const QString &symbol,
                       const QString &tooltip,
                       int size = 32) -> QToolButton * {
        QToolButton *btn = new QToolButton(nav);
        btn->setText(symbol);
        btn->setToolTip(tooltip);
        btn->setFixedSize(size, size);
        btn->setStyleSheet(
            QString(
                "QToolButton {"
                "  color: %1;"
                "  background: transparent;"
                "  border: none;"
                "  border-radius: %2px;"
                "  font-size: 15px;"
                "  font-family: 'Segoe UI Symbol', 'Segoe UI', sans-serif;"
                "}"
                "QToolButton:hover    { background: %3; }"
                "QToolButton:pressed  { background: %4; }"
                "QToolButton:disabled { color: #5f6368; }"
            ).arg(kTextDim, QString::number(size / 2), kHover, kBorder)
        );
        return btn;
    };

    m_backBtn    = makeBtn("←", tr("Back (Alt+←)"));
    m_forwardBtn = makeBtn("→", tr("Forward (Alt+→)"));
    m_reloadBtn  = makeBtn("⟳", tr("Reload (F5)"));

    connect(m_backBtn,    &QToolButton::clicked, this,
            [this]() { if (auto *v = currentWebView()) v->back(); });
    connect(m_forwardBtn, &QToolButton::clicked, this,
            [this]() { if (auto *v = currentWebView()) v->forward(); });
    connect(m_reloadBtn,  &QToolButton::clicked, this,
            [this]() { if (auto *v = currentWebView()) v->reload(); });

    // ── Address bar (omnibox) ────────────────────────────────────────────────
    m_addressBar = new QLineEdit(nav);
    m_addressBar->setPlaceholderText(tr("Search Google or type a URL"));
    m_addressBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_addressBar->setStyleSheet(
        QString(
            "QLineEdit {"
            "  background: %1;"
            "  color: %2;"
            "  border: 1px solid %3;"
            "  border-radius: 16px;"
            "  padding: 5px 16px;"
            "  font-family: '%4';"
            "  font-size: 13px;"
            "  selection-background-color: %5;"
            "}"
            "QLineEdit:focus {"
            "  border-color: %5;"
            "  background: %6;"
            "}"
        ).arg(kOmnibox, kText, kBorder, kFontFamily, kAccent, kFrame)
    );

    // ── Hamburger menu ( ⋮ ) ────────────────────────────────────────────────
    QToolButton *menuBtn = makeBtn("⋮", tr("Customize and control"), 32);
    menuBtn->setPopupMode(QToolButton::InstantPopup);
    menuBtn->setStyleSheet(
        menuBtn->styleSheet() +
        "QToolButton::menu-indicator { image: none; }"
    );

    QMenu *hamburger = new QMenu(menuBtn);
    hamburger->setStyleSheet(
        QString(
            "QMenu {"
            "  background: %1;"
            "  color: %2;"
            "  border: 1px solid %3;"
            "  border-radius: 8px;"
            "  padding: 4px 0px;"
            "  font-family: '%4';"
            "  font-size: 12px;"
            "}"
            "QMenu::item {"
            "  padding: 8px 32px 8px 16px;"
            "}"
            "QMenu::item:selected {"
            "  background: %5;"
            "  border-radius: 0px;"
            "}"
            "QMenu::separator {"
            "  height: 1px;"
            "  background: %3;"
            "  margin: 4px 0px;"
            "}"
        ).arg(kSurface, kText, kBorder, kFontFamily, kHover)
    );
    buildHamburgerMenu(hamburger);
    menuBtn->setMenu(hamburger);

    hl->addWidget(m_backBtn);
    hl->addWidget(m_forwardBtn);
    hl->addWidget(m_reloadBtn);
    hl->addSpacing(8);
    hl->addWidget(m_addressBar);
    hl->addSpacing(4);
    hl->addWidget(menuBtn);
}

// ─────────────────────────────────────────────────────────────────────────────
//  Hamburger menu items
// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::buildHamburgerMenu(QMenu *menu)
{
    QAction *newTab = menu->addAction(tr("New tab                Ctrl+T"));
    connect(newTab, &QAction::triggered, this, [this]() { addNewTab(); });

    QAction *newWindow=menu->addAction(tr("New Window            Ctrl+N"));
    connect(newWindow,&QAction::triggered,this,[this](){addNewWindow();});

    QAction *incognito = menu->addAction(tr("New incognito tab   Ctrl+I"));
    connect(incognito, &QAction::triggered, this, [this]() { addNewIncognitoTab(); });

    QAction *newIncognitoWindow= menu->addAction(tr("New Incognito window Ctrl+Shift+N"));
    connect(newIncognitoWindow,&QAction::triggered,this,[this](){addNewIncognitoWindow();});

    QAction *closeTab = menu->addAction(tr("Close tab              Ctrl+W"));
    connect(closeTab, &QAction::triggered, this, [this]() {
        onTabCloseRequested(m_tabBar->currentIndex());
    });

    menu->addSeparator();

    QAction *zoomIn  = menu->addAction(tr("Zoom in                 Ctrl++"));
    QAction *zoomOut = menu->addAction(tr("Zoom out               Ctrl+-"));
    connect(zoomIn,  &QAction::triggered, this, [this]() {
        if (auto *v = currentWebView()) v->setZoomFactor(v->zoomFactor() + 0.1);
    });
    connect(zoomOut, &QAction::triggered, this, [this]() {
        if (auto *v = currentWebView()) v->setZoomFactor(v->zoomFactor() - 0.1);
    });

    menu->addSeparator();

    QAction *downloads = menu->addAction(tr("Downloads"));
    connect(downloads, &QAction::triggered, this, [this]() {
        m_downloadManager->show();
    });

    menu->addSeparator();
    QAction *apiTester = menu->addAction(tr("REST API Tester"));

    connect(
        apiTester,
        &QAction::triggered,
        this,
        &MainWindow::addApiTesterTab
        );

    menu->addSeparator();

    QAction *history = menu->addAction(tr("History                  Ctrl+H"));
    connect(history, &QAction::triggered, this, [this]() {
        HistoryDialog *dialog = new HistoryDialog(m_historyManager, this);
        connect(dialog, &HistoryDialog::urlSelected,
                this, [this](const QUrl &url) { addNewTab(url); });
        dialog->exec();
    });

    QAction *bookmarks = menu->addAction(tr("Bookmarks"));
    connect(bookmarks, &QAction::triggered, this, [this]() {
        BookmarkDialog *dialog = new BookmarkDialog(m_bookMarkManager, this);
        connect(dialog, &BookmarkDialog::urlSelected,
                this, [this](const QUrl &url) { addNewTab(url); });
        dialog->exec();
    });

    QAction *addBookmark = menu->addAction(tr("Bookmark This Page"));
    connect(addBookmark, &QAction::triggered, this, [this]() {
        if (auto *v = currentWebView())
            m_bookMarkManager->addBookMark(v->url(), v->title());
    });

    menu->addSeparator();

    QAction *theme = menu->addAction(tr("Toggle Theme"));
    connect(theme,
            &QAction::triggered,
            this,
            &MainWindow::toggleTheme);

    QAction *quit = menu->addAction(tr("Quit                       Ctrl+Q"));
    connect(quit, &QAction::triggered, this, &QWidget::close);

}

// ─────────────────────────────────────────────────────────────────────────────
//  Status bar
// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::setupStatusBar()
{
    statusBar()->setStyleSheet(
        QString(
            "QStatusBar {"
            "  background: %1;"
            "  color: %2;"
            "  border-top: 1px solid %3;"
            "  font-family: '%4';"
            "  font-size: 11px;"
            "}"
        ).arg(kToolbar, kTextDim, kBorder, kFontFamily)
    );
    statusBar()->showMessage(tr("Ready"));

    m_progressBar = new QProgressBar(this);
    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(0);
    m_progressBar->setTextVisible(false);
    m_progressBar->setFixedSize(150, 10);
    m_progressBar->setStyleSheet(
        QString(
            "QProgressBar {"
            "  background: %1; border: none; border-radius: 5px;"
            "}"
            "QProgressBar::chunk {"
            "  background: %2; border-radius: 5px;"
            "}"
        ).arg(kBorder, kAccent)
    );
    statusBar()->addPermanentWidget(m_progressBar);

    m_progressLabel = new QLabel(tr("0%"), this);
    m_progressLabel->setFixedWidth(36);
    m_progressLabel->setStyleSheet(
        QString("color: %1; font-size: 11px;").arg(kTextDim)
    );
    statusBar()->addPermanentWidget(m_progressLabel);

    m_networkStatusLabel = new QLabel("Checking...", this);
    statusBar()->addPermanentWidget(m_networkStatusLabel);

}

// ─────────────────────────────────────────────────────────────────────────────
//  Tab management
// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::createTab(QWebEngineProfile *profile,
                           const QUrl        &url,
                           const QString     &prefix)
{
    QWebEngineView    *webView = new QWebEngineView(this);
    CustomBrowserPage *page    = new CustomBrowserPage(profile, webView);

    page->settings()->setAttribute(QWebEngineSettings::FullScreenSupportEnabled,true);

    // Certificate errors
    connect(page, &QWebEnginePage::certificateError, this,
            [this](QWebEngineCertificateError error) {
                auto reply = QMessageBox::warning(
                    this, tr("HTTPS Certificate Warning"),
                    QString(tr("The certificate for\n\n%1\n\nis invalid.\n\nContinue anyway?"))
                        .arg(error.url().toString()),
                    QMessageBox::Yes | QMessageBox::No
                );
                if (reply == QMessageBox::Yes)
                    error.acceptCertificate();
                else
                    error.rejectCertificate();
            });

    // Permission prompts
    connect(page, &QWebEnginePage::permissionRequested, this,
            [this](QWebEnginePermission permission) {
                QString name;
                switch (permission.permissionType()) {
                case QWebEnginePermission::PermissionType::Geolocation:       name = tr("Location");      break;
                case QWebEnginePermission::PermissionType::MediaAudioCapture: name = tr("Microphone");    break;
                case QWebEnginePermission::PermissionType::MediaVideoCapture: name = tr("Camera");        break;
                case QWebEnginePermission::PermissionType::Notifications:     name = tr("Notifications"); break;
                default:                                                      name = tr("Permission");    break;
                }
                auto reply = QMessageBox::question(
                    this, tr("Permission Request"),
                    QString(tr("%1 wants access to your %2.\nAllow?"))
                        .arg(permission.origin().host(), name)
                );
                if (reply == QMessageBox::Yes) permission.grant();
                else                           permission.deny();
            });

    // Popup blocking
    connect(page, &CustomBrowserPage::popupBlocked,
            this, &MainWindow::onPopupBlocked);

    webView->setPage(page);

    connect(
        page,
        &QWebEnginePage::fullScreenRequested,
        this,
        [this](QWebEngineFullScreenRequest request)
        {
            request.accept();

            if (request.toggleOn())
            {
                menuBar()->hide();
                statusBar()->hide();

                m_tabBar->hide();
                m_navWidget->hide();

                showFullScreen();
            }
            else
            {
                showNormal();

                m_tabBar->show();
                m_navWidget->show();

                statusBar()->show();
            }
        }
        );

    // Add to stack & tab bar (indices stay in sync)
    int stackIdx = m_webStack->addWidget(webView);
    int tabIdx   = m_tabBar->addTab(prefix + tr("New Tab"));
    m_webStack->setCurrentIndex(stackIdx);
    m_tabBar->setCurrentIndex(tabIdx);

    // Title
    connect(webView, &QWebEngineView::titleChanged,
            this, [this, webView, prefix](const QString &title) {
                int i = m_webStack->indexOf(webView);
                if (i >= 0)
                    m_tabBar->setTabText(i, prefix + (title.isEmpty() ? tr("New Tab") : title));
                if (m_webStack->currentWidget() == webView)
                    setWindowTitle(title.isEmpty() ? tr("Qt Browser") : title);
            });

    // Favicon
    connect(webView, &QWebEngineView::iconChanged,
            this, [this, webView](const QIcon &icon) {
                int i = m_webStack->indexOf(webView);
                if (i >= 0)
                    m_tabBar->setTabIcon(i, icon);
            });

    // URL sync — clear address bar for internal new-tab pages
    connect(webView, &QWebEngineView::urlChanged,
            this, [this, webView](const QUrl &newUrl) {
                if (m_webStack->currentWidget() == webView) {
                    if (newUrl.scheme() == "qrc")
                        m_addressBar->clear();
                    else
                        m_addressBar->setText(newUrl.toString());
                }
            });

    // Progress
    connect(webView, &QWebEngineView::loadProgress,
            this, [this, webView](int progress) {
                if (m_webStack->currentWidget() == webView) {
                    m_progressBar->setValue(progress);
                    m_progressLabel->setText(QString::number(progress) + "%");
                }
            });

    // Status
    connect(webView, &QWebEngineView::loadStarted,
            this, [this, webView]() {
                if (m_webStack->currentWidget() == webView)
                    statusBar()->showMessage(tr("Loading…"));
            });

    webView->setUrl(url);

    connect(
        webView,
        &QWebEngineView::loadFinished,
        this,
        [this, webView](bool ok)
        {
            if (!ok)
                return;

            m_historyManager->addHistory(
                webView->url(),
                webView->title()
                );
        }
        );

}

void MainWindow::addNewTab(const QUrl &url)
{
    createTab(QWebEngineProfile::defaultProfile(), url);
}

void MainWindow::addNewIncognitoTab(const QUrl &url)
{
    createTab(new QWebEngineProfile(this), url, QString::fromUtf8("🕶 "));
}

void MainWindow::addNewWindow()
{
    MainWindow *window= new MainWindow(false);

    window->setGeometry(this->geometry());

    window->show();
}

void MainWindow::addNewIncognitoWindow()
{
    MainWindow *window = new MainWindow(true);

    window->setGeometry(this->geometry());

    window->show();
}

void MainWindow::onTabCloseRequested(int index)
{
    if (m_tabBar->count() <= 1)
        return;

    QWidget *page = m_webStack->widget(index);

    m_webStack->removeWidget(page);
    m_tabBar->removeTab(index);

    delete page;

}

void MainWindow::onCurrentTabChanged(int index)
{
    if (index < 0 || index >= m_webStack->count()) return;
    m_webStack->setCurrentIndex(index);

    for (int i = 0; i < m_webStack->count(); i++)
    {
        auto *view =
            qobject_cast<QWebEngineView*>(
                m_webStack->widget(i));

        if (!view)
            continue;

        view->setUpdatesEnabled(i == index);
    }

    QWebEngineView *view = currentWebView();
    if (!view){
        m_addressBar->clear();
        setWindowTitle("REST API Tester");
        return;
    }

    if (view->url().scheme() == "qrc")
        m_addressBar->clear();
    else
        m_addressBar->setText(view->url().toString());
    setWindowTitle(view->title().isEmpty() ? tr("Qt Browser") : view->title());
    statusBar()->showMessage(tr("Ready"));
}

void MainWindow::onAddressEntered()
{
    QString input = m_addressBar->text().trimmed();
    if (input.isEmpty()) return;

    QUrl url;

    // Detect if it's a URL or a search term
    if (input.startsWith("http://") || input.startsWith("https://") ||
        input.startsWith("qrc://") || (input.contains(".") && !input.contains(" ")))
    {
        url = QUrl::fromUserInput(input);
    }
    else
    {
        // Treat as search query
        url = QUrl("https://www.google.com/search?q=" +
                   QUrl::toPercentEncoding(input));
    }

    if (auto *v = currentWebView())
        v->setUrl(url);
}

QWebEngineView* MainWindow::currentWebView()
{
    return qobject_cast<QWebEngineView*>(m_webStack->currentWidget());
}

// ─────────────────────────────────────────────────────────────────────────────
//  Popup-blocked notification bar
// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::onPopupBlocked(const QUrl &url)
{
    if (m_notificationBar) {
        m_notificationBar->deleteLater();
        m_notificationBar = nullptr;
    }

    m_notificationBar = new QWidget(this);
    m_notificationBar->setStyleSheet(
        QString(
            "background: %1;"
            "border-bottom: 1px solid %2;"
            "color: %3;"
            "font-family: '%4';"
        ).arg(kSurface, kBorder, kText, kFontFamily)
    );
    m_notificationBar->setFixedHeight(36);

    QHBoxLayout *layout = new QHBoxLayout(m_notificationBar);
    layout->setContentsMargins(12, 4, 12, 4);

    QLabel      *label      = new QLabel(tr("Pop-up blocked"), m_notificationBar);
    QPushButton *openBtn    = new QPushButton(tr("Open anyway"), m_notificationBar);
    QPushButton *dismissBtn = new QPushButton(tr("Dismiss"),     m_notificationBar);

    openBtn->setStyleSheet(
        QString(
            "background: %1; color: #202124; border: none; border-radius: 4px;"
            "padding: 4px 12px; font-size: 12px; font-weight: 600;"
        ).arg(kAccent)
    );
    dismissBtn->setStyleSheet(
        QString(
            "background: %1; color: %2; border: none; border-radius: 4px;"
            "padding: 4px 12px; font-size: 12px;"
        ).arg(kHover, kText)
    );

    layout->addWidget(label);
    layout->addStretch();
    layout->addWidget(openBtn);
    layout->addWidget(dismissBtn);

    connect(openBtn, &QPushButton::clicked, this, [this, url]() {
        addNewTab(url);
        if (m_notificationBar) { m_notificationBar->deleteLater(); m_notificationBar = nullptr; }
    });
    connect(dismissBtn, &QPushButton::clicked, this, [this]() {
        if (m_notificationBar) { m_notificationBar->deleteLater(); m_notificationBar = nullptr; }
    });

    int navBottom = m_navWidget->y() + m_navWidget->height();
    m_notificationBar->setParent(this);
    m_notificationBar->setGeometry(0, navBottom, width(), 36);
    m_notificationBar->show();
    m_notificationBar->raise();
}

void MainWindow::addApiTesterTab()
{
    ApiTesterWidget *tester = new ApiTesterWidget(this);

    m_webStack->addWidget(tester);

    int tabIndex = m_tabBar->addTab("REST API");

    m_tabBar->setCurrentIndex(tabIndex);   // onCurrentTabChanged() updates m_webStack
}

void MainWindow::toggleTheme()
{
    if (m_darkTheme)
    {
        qApp->setStyleSheet("");
    }
    else
    {
        setStyleSheet(
            QString("QMainWindow { background: %1; }")
                .arg(kFrame)
            );
    }

    m_darkTheme = !m_darkTheme;
}