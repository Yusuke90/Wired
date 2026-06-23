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

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle(tr("Qt Browser"));
    setupToolBar();
    setupMenuBar();
    setupStatusBar();

    // Module 4 — Tab widget replaces single web view
    m_tabWidget = new QTabWidget(this);
    m_tabWidget->setTabsClosable(true);
    m_tabWidget->setMovable(true);
    setCentralWidget(m_tabWidget);

    connect(
        m_tabWidget,
        &QTabWidget::tabCloseRequested,
        this,
        &MainWindow::onTabCloseRequested
        );

    QToolButton *newTabButton = new QToolButton(this);
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

    m_tabWidget->setCornerWidget(newTabButton);

    connect(
        m_tabWidget,
        &QTabWidget::currentChanged,
        this,
        [this](int)
        {
            QWebEngineView *view = currentWebView();

            if (!view)
                return;

            m_addressBar->setText(view->url().toString());

            setWindowTitle(view->title());

            statusBar()->showMessage("Ready");
        }
        );

    // Wire address bar
    connect(m_addressBar, &QLineEdit::returnPressed,
            this, &MainWindow::onAddressEntered);

    // Open first tab at startup
    addNewTab(QUrl("https://www.google.com"));
}

void MainWindow::addNewTab(const QUrl &url)
{
    QWebEngineView *webView = new QWebEngineView(this);
    webView->setUrl(url);

    int index = m_tabWidget->addTab(webView, tr("New Tab"));
    m_tabWidget->setCurrentIndex(index);

    // Update tab title when page title changes
    connect(webView, &QWebEngineView::titleChanged,
            this, [this, webView](const QString &title) {
                int i = m_tabWidget->indexOf(webView);
                if (i >= 0)
                    m_tabWidget->setTabText(i, title.isEmpty() ? tr("New Tab") : title);
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

    m_backAction    = m_toolBar->addAction(style()->standardIcon(QStyle::SP_ArrowBack), tr("Back"));
    m_forwardAction = m_toolBar->addAction(style()->standardIcon(QStyle::SP_ArrowForward), tr("Forward"));
    m_reloadAction  = m_toolBar->addAction(style()->standardIcon(QStyle::SP_BrowserReload), tr("Reload"));
    m_homeAction    = m_toolBar->addAction(style()->standardIcon(QStyle::SP_DirHomeIcon), tr("Home"));

    m_backAction->setEnabled(true);
    m_forwardAction->setEnabled(true);

    m_addressBar = new QLineEdit(this);
    m_addressBar->setPlaceholderText(tr("Search or enter address"));
    m_toolBar->addWidget(m_addressBar);

    // Wire Back/Forward/Reload to active tab
    connect(m_backAction, &QAction::triggered, this, [this]() {
        QWebEngineView *v = currentWebView();
        if (v) v->back();
    });

    connect(m_forwardAction, &QAction::triggered, this, [this]() {
        QWebEngineView *v = currentWebView();
        if (v) v->forward();
    });

    connect(m_reloadAction, &QAction::triggered, this, [this]() {
        QWebEngineView *v = currentWebView();
        if (v) v->reload();
    });
}

QWebEngineView* MainWindow::currentWebView()
{
    return qobject_cast<QWebEngineView*>(
        m_tabWidget->currentWidget()
        );
}

void MainWindow::setupMenuBar()
{
    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
    QAction *newTabAction = fileMenu->addAction(tr("New Tab"));
    newTabAction->setShortcut(QKeySequence::AddTab);
    connect(newTabAction, &QAction::triggered,
            this, [this]() { addNewTab(); });

    QAction *closeTabAction = fileMenu->addAction(tr("Close Tab"));
    closeTabAction->setShortcut(QKeySequence("Ctrl+W"));
    connect(closeTabAction, &QAction::triggered,
            this, [this]() { onTabCloseRequested(m_tabWidget->currentIndex()); });

    fileMenu->addSeparator();
    QAction *quitAction = fileMenu->addAction(tr("Quit"));
    quitAction->setShortcut(QKeySequence::Quit);
    connect(quitAction, &QAction::triggered, this, &QWidget::close);

    QMenu *editMenu = menuBar()->addMenu(tr("&Edit"));
    editMenu->addAction(tr("Cut"));
    editMenu->addAction(tr("Copy"));
    editMenu->addAction(tr("Paste"));

    QMenu *viewMenu = menuBar()->addMenu(tr("&View"));
    viewMenu->addAction(tr("Zoom In"));
    viewMenu->addAction(tr("Zoom Out"));
    viewMenu->addAction(tr("Reset Zoom"));
    viewMenu->addAction(tr("Toggle Full Screen"));

    QMenu *bookmarksMenu = menuBar()->addMenu(tr("&Bookmarks"));
    bookmarksMenu->addAction(tr("Add Bookmark"));
    bookmarksMenu->addAction(tr("Show All Bookmarks"));

    QMenu *historyMenu = menuBar()->addMenu(tr("Hi&story"));
    historyMenu->addAction(tr("Show History"));
    historyMenu->addAction(tr("Clear History"));

    QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(tr("About"));
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