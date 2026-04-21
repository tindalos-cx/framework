#include "MainWindow.h"
#include "PluginManagerDialog.h"
#include <PluginFramework/IHost.h>
#include <PluginFramework/PluginManager.h>
#include <QVBoxLayout>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QMessageBox>
#include <QDateTime>

MainWindow::MainWindow(std::shared_ptr<PluginFramework::IHost> host,
                       std::shared_ptr<PluginFramework::PluginManager> pluginManager,
                       QWidget* parent)
    : QMainWindow(parent)
    , m_host(std::move(host))
    , m_pluginManager(std::move(pluginManager)) {
    setupUI();
    createActions();
    createMenus();
    createToolbars();
    createStatusBar();

    // 记录启动日志
    m_logWidget->append(QString("[%1] 宿主应用已启动")
        .arg(QDateTime::currentDateTime().toString(Qt::ISODate)));
}

MainWindow::~MainWindow() = default;

void MainWindow::setupUI() {
    setWindowTitle("Plugin Host Application");
    resize(1024, 768);

    // 创建日志窗口
    m_logWidget = new QTextEdit(this);
    m_logWidget->setReadOnly(true);
    m_logWidget->setFont(QFont("Consolas", 9));
    setCentralWidget(m_logWidget);
}

void MainWindow::createActions() {
    // 插件管理
    m_pluginManagerAction = new QAction(tr("插件管理"), this);
    m_pluginManagerAction->setShortcut(QKeySequence("Ctrl+P"));
    m_pluginManagerAction->setStatusTip(tr("打开插件管理对话框"));
    connect(m_pluginManagerAction, &QAction::triggered, this, &MainWindow::onOpenPluginManager);

    // 重载插件
    m_reloadPluginsAction = new QAction(tr("重载插件"), this);
    m_reloadPluginsAction->setShortcut(QKeySequence("Ctrl+R"));
    m_reloadPluginsAction->setStatusTip(tr("重新加载所有已启用的插件"));
    connect(m_reloadPluginsAction, &QAction::triggered, this, &MainWindow::onReloadPlugins);

    // 退出
    m_exitAction = new QAction(tr("退出"), this);
    m_exitAction->setShortcut(QKeySequence("Ctrl+Q"));
    m_exitAction->setStatusTip(tr("退出应用程序"));
    connect(m_exitAction, &QAction::triggered, this, &QWidget::close);

    // 关于
    m_aboutAction = new QAction(tr("关于"), this);
    m_aboutAction->setStatusTip(tr("关于本程序"));
    connect(m_aboutAction, &QAction::triggered, this, &MainWindow::onAbout);
}

void MainWindow::createMenus() {
    // 文件菜单
    QMenu* fileMenu = menuBar()->addMenu(tr("文件"));
    fileMenu->addAction(m_reloadPluginsAction);
    fileMenu->addSeparator();
    fileMenu->addAction(m_exitAction);

    // 插件菜单
    QMenu* pluginMenu = menuBar()->addMenu(tr("插件"));
    pluginMenu->addAction(m_pluginManagerAction);

    // 帮助菜单
    QMenu* helpMenu = menuBar()->addMenu(tr("帮助"));
    helpMenu->addAction(m_aboutAction);
}

void MainWindow::createToolbars() {
    // 主工具栏
    QToolBar* mainToolbar = addToolBar(tr("主工具栏"));
    mainToolbar->setMovable(false);
    mainToolbar->addAction(m_pluginManagerAction);
    mainToolbar->addAction(m_reloadPluginsAction);
}

void MainWindow::createStatusBar() {
    statusBar()->showMessage(tr("就绪"));
}

void MainWindow::onOpenPluginManager() {
    PluginManagerDialog dialog(m_pluginManager, this);
    dialog.exec();
}

void MainWindow::onReloadPlugins() {
    statusBar()->showMessage(tr("正在重载插件..."));
    m_logWidget->append(QString("[%1] 正在重载插件...")
        .arg(QDateTime::currentDateTime().toString(Qt::ISODate)));

    // TODO: 实现重载逻辑

    statusBar()->showMessage(tr("插件重载完成"));
}

void MainWindow::onAbout() {
    QMessageBox::about(this, tr("关于"),
        tr("<h3>Plugin Host Application</h3>"
           "<p>版本: 1.0.0</p>"
           "<p>基于 C++ DLL 的插件框架示例应用</p>"));
}
