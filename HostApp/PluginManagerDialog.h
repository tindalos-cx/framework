#pragma once

#include <QDialog>
#include <QTableWidget>
#include <QPushButton>
#include <QLabel>
#include <memory>

namespace PluginFramework {
    class PluginManager;
}

class PluginManagerDialog : public QDialog {
    Q_OBJECT

public:
    explicit PluginManagerDialog(std::shared_ptr<PluginFramework::PluginManager> pluginManager,
                                  QWidget* parent = nullptr);
    ~PluginManagerDialog();

private slots:
    void onRefresh();
    void onLoadPlugin();
    void onUnloadPlugin();
    void onEnablePlugin();
    void onDisablePlugin();

private:
    void setupUI();
    void refreshPluginList();

    std::shared_ptr<PluginFramework::PluginManager> m_pluginManager;

    QTableWidget* m_pluginTable;
    QPushButton* m_refreshBtn;
    QPushButton* m_loadBtn;
    QPushButton* m_unloadBtn;
    QPushButton* m_enableBtn;
    QPushButton* m_disableBtn;
    QPushButton* m_closeBtn;
    QLabel* m_statusLabel;
};
