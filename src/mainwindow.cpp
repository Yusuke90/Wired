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

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle(tr("Qt Browser"));
    setupToolBar();
    setupMenuBar();
    setupStatusBar();

    // Module 3 — WebEngine view
    m_webView = new QWebEngineView(this);
    setCentralWidget(m_webView);
    m_webView->setUrl(QUrl("https://www.google.com"));

    // Wire address bar
    connect(m_addressBar, &QLineEdit::returnPressed,
            this, &MainWindow::onAddressEntered);

    connect(m_backAction,&QAction::triggered,
            m_webView,&QWebEngineView::back);

    connect(m_forwardAction,&QAction::triggered,
            m_webView,&QWebEngineView::forward);

    connect(m_reloadAction,&QAction::triggered,
            m_webView,&QWebEngineView::reload);

    connect(m_webView,&QWebEngineView::loadProgress,
            m_progressBar,&QProgressBar::setValue);

    connect(
        m_webView,
        &QWebEngineView::loadProgress,
        this,
        [this](int progress)
        {
            m_progressLabel->setText(QString::number(progress) + "%");
        }
        );

    connect(m_webView
            ,&QWebEngineView::loadStarted,
            this,
            [this]()
            {
                statusBar()->showMessage("Loading..");
            }
            );

    connect(
        m_webView,
        &QWebEngineView::loadFinished,
        this,
        [this](bool)
        {
            statusBar()->showMessage("Ready");
        }
        );

    connect(m_webView,&QWebEngineView::titleChanged,
            this,&QMainWindow::setWindowTitle);
}

void MainWindow::setupToolBar()
{
    m_toolBar = addToolBar(tr("Navigation"));
    m_toolBar->setMovable(false);

    m_backAction    = m_toolBar->addAction(style()->standardIcon(QStyle::SP_ArrowBack), tr("Back"));
    m_forwardAction = m_toolBar->addAction(style()->standardIcon(QStyle::SP_ArrowForward), tr("Forward"));
    m_reloadAction  = m_toolBar->addAction(style()->standardIcon(QStyle::SP_BrowserReload), tr("Reload"));
    m_homeAction    = m_toolBar->addAction(style()->standardIcon(QStyle::SP_DirHomeIcon), tr("Home"));

    // No QWebEngineView yet (that's Module 3), so these can't do anything real.
    // Disabled rather than wired to a no-op, so the UI honestly reflects state.
    m_backAction->setEnabled(true);
    m_forwardAction->setEnabled(true);

    m_addressBar = new QLineEdit(this);
    m_addressBar->setPlaceholderText(tr("Search or enter address"));
    m_toolBar->addWidget(m_addressBar);

    // Module 3: connect m_addressBar->returnPressed() to QWebEngineView::load(QUrl)
}

void MainWindow::setupMenuBar()
{
    // File
    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
    QAction *newTabAction = fileMenu->addAction(tr("New Tab"));
    newTabAction->setShortcut(QKeySequence::AddTab); // Ctrl+T
    QAction *closeTabAction = fileMenu->addAction(tr("Close Tab"));
    closeTabAction->setShortcut(QKeySequence::Close); // Ctrl+W
    fileMenu->addSeparator();
    QAction *quitAction = fileMenu->addAction(tr("Quit"));
    quitAction->setShortcut(QKeySequence::Quit);
    connect(quitAction, &QAction::triggered, this, &QWidget::close);

    // Edit
    QMenu *editMenu = menuBar()->addMenu(tr("&Edit"));
    editMenu->addAction(tr("Cut"));
    editMenu->addAction(tr("Copy"));
    editMenu->addAction(tr("Paste"));

    // View
    QMenu *viewMenu = menuBar()->addMenu(tr("&View"));
    viewMenu->addAction(tr("Zoom In"));
    viewMenu->addAction(tr("Zoom Out"));
    viewMenu->addAction(tr("Reset Zoom"));
    viewMenu->addAction(tr("Toggle Full Screen"));

    // Bookmarks
    QMenu *bookmarksMenu = menuBar()->addMenu(tr("&Bookmarks"));
    bookmarksMenu->addAction(tr("Add Bookmark"));
    bookmarksMenu->addAction(tr("Show All Bookmarks"));

    // History
    QMenu *historyMenu = menuBar()->addMenu(tr("Hi&story"));
    historyMenu->addAction(tr("Show History"));
    historyMenu->addAction(tr("Clear History"));

    // Help
    QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(tr("About"));
}

void MainWindow::setupStatusBar()
{
    statusBar()->showMessage(tr("Ready"));

    m_progressBar = new QProgressBar(this);
    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(50);
    m_progressBar->setTextVisible(false);
    m_progressBar->setFixedSize(150, 14);
    statusBar()->addPermanentWidget(m_progressBar);

    m_progressLabel = new QLabel(tr("50%"), this);
    m_progressLabel->setFixedWidth(32);
    statusBar()->addPermanentWidget(m_progressLabel);
}
void MainWindow::onAddressEntered()
{
    QUrl url = QUrl::fromUserInput(m_addressBar->text());
    m_webView->setUrl(url);
}