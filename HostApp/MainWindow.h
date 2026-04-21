#pragma once

#include <QMainWindow>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QAction>
#include <QTextEdit>
#include <memory>

namespace PluginFramework {
    class IHost;
    class PluginManager;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(std::shared_ptr<PluginFramework::IHost> host,
                        std::shared_ptr<PluginFramework::PluginManager> pluginManager,
                        QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void onOpenPluginManager();
    void onReloadPlugins();
    void onAbout();

private:
    void setupUI();
    void createActions();
    void createMenus();
    void createToolbars();
    void createStatusBar();

    std::shared_ptr<PluginFramework::IHost> m_host;
    std::shared_ptr<PluginFramework::PluginManager> m_pluginManager;
    QTextEdit* m_logWidget;

    // 动作
    QAction* m_pluginManagerAction;
    QAction* m_reloadPluginsAction;
    QAction* m_exitAction;
    QAction* m_aboutAction;
};
