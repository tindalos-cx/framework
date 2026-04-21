#include "PluginManagerDialog.h"
#include <PluginFramework/PluginManager.h>
#include <PluginFramework/PluginTypes.h>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QFileDialog>

PluginManagerDialog::PluginManagerDialog(std::shared_ptr<PluginFramework::PluginManager> pluginManager,
                                           QWidget* parent)
    : QDialog(parent)
    , m_pluginManager(std::move(pluginManager)) {
    setupUI();
    refreshPluginList();

    setWindowTitle("插件管理");
    resize(800, 500);
}

PluginManagerDialog::~PluginManagerDialog() = default;

void PluginManagerDialog::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // 插件列表
    m_pluginTable = new QTableWidget(this);
    m_pluginTable->setColumnCount(5);
    m_pluginTable->setHorizontalHeaderLabels({"ID", "名称", "版本", "状态", "描述"});
    m_pluginTable->horizontalHeader()->setStretchLastSection(true);
    m_pluginTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_pluginTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_pluginTable->setAlternatingRowColors(true);
    mainLayout->addWidget(m_pluginTable);

    // 状态标签
    m_statusLabel = new QLabel(this);
    mainLayout->addWidget(m_statusLabel);

    // 按钮栏
    QHBoxLayout* btnLayout = new QHBoxLayout();

    m_refreshBtn = new QPushButton("刷新", this);
    connect(m_refreshBtn, &QPushButton::clicked, this, &PluginManagerDialog::onRefresh);
    btnLayout->addWidget(m_refreshBtn);

    m_loadBtn = new QPushButton("加载", this);
    connect(m_loadBtn, &QPushButton::clicked, this, &PluginManagerDialog::onLoadPlugin);
    btnLayout->addWidget(m_loadBtn);

    m_unloadBtn = new QPushButton("卸载", this);
    connect(m_unloadBtn, &QPushButton::clicked, this, &PluginManagerDialog::onUnloadPlugin);
    btnLayout->addWidget(m_unloadBtn);

    btnLayout->addSpacing(20);

    m_enableBtn = new QPushButton("启用", this);
    connect(m_enableBtn, &QPushButton::clicked, this, &PluginManagerDialog::onEnablePlugin);
    btnLayout->addWidget(m_enableBtn);

    m_disableBtn = new QPushButton("禁用", this);
    connect(m_disableBtn, &QPushButton::clicked, this, &PluginManagerDialog::onDisablePlugin);
    btnLayout->addWidget(m_disableBtn);

    btnLayout->addStretch();

    m_closeBtn = new QPushButton("关闭", this);
    connect(m_closeBtn, &QPushButton::clicked, this, &QDialog::accept);
    btnLayout->addWidget(m_closeBtn);

    mainLayout->addLayout(btnLayout);
}

void PluginManagerDialog::refreshPluginList() {
    m_pluginTable->setRowCount(0);

    auto plugins = m_pluginManager->GetAllPlugins();

    for (const auto& plugin : plugins) {
        const auto& metadata = plugin->GetMetadata();

        int row = m_pluginTable->rowCount();
        m_pluginTable->insertRow(row);

        m_pluginTable->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(metadata.id)));
        m_pluginTable->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(metadata.name)));
        m_pluginTable->setItem(row, 2, new QTableWidgetItem(QString::fromStdString(metadata.version)));

        QString stateStr;
        switch (plugin->GetMetadata().id.empty() ? PluginFramework::PluginState::Unknown : PluginFramework::PluginState::Running) {
            case PluginFramework::PluginState::Unknown: stateStr = "未知"; break;
            case PluginFramework::PluginState::Discovered: stateStr = "已发现"; break;
            case PluginFramework::PluginState::Loaded: stateStr = "已加载"; break;
            case PluginFramework::PluginState::Ready: stateStr = "就绪"; break;
            case PluginFramework::PluginState::Running: stateStr = "运行中"; break;
            case PluginFramework::PluginState::Error: stateStr = "错误"; break;
            case PluginFramework::PluginState::Unloaded: stateStr = "已卸载"; break;
        }
        m_pluginTable->setItem(row, 3, new QTableWidgetItem(stateStr));

        m_pluginTable->setItem(row, 4, new QTableWidgetItem(QString::fromStdString(metadata.description)));
    }

    m_statusLabel->setText(QString("共 %1 个插件").arg(plugins.size()));
}

void PluginManagerDialog::onRefresh() {
    refreshPluginList();
}

void PluginManagerDialog::onLoadPlugin() {
    QString filePath = QFileDialog::getOpenFileName(
        this, "选择插件", "", "插件文件 (*.dll *.so *.dylib)");
    
    if (filePath.isEmpty()) return;

    auto error = m_pluginManager->LoadPlugin(filePath.toStdString());
    if (error != PluginFramework::PluginError::Success) {
        QMessageBox::warning(this, "错误", "加载插件失败");
    }
    refreshPluginList();
}

void PluginManagerDialog::onUnloadPlugin() {
    int currentRow = m_pluginTable->currentRow();
    if (currentRow < 0) {
        QMessageBox::warning(this, "警告", "请先选择一个插件");
        return;
    }

    QString pluginId = m_pluginTable->item(currentRow, 0)->text();
    auto error = m_pluginManager->UnloadPlugin(pluginId.toStdString());
    if (error != PluginFramework::PluginError::Success) {
        QMessageBox::warning(this, "错误", "卸载插件失败");
    }
    refreshPluginList();
}

void PluginManagerDialog::onEnablePlugin() {
    QMessageBox::information(this, "提示", "启用插件功能待实现");
}

void PluginManagerDialog::onDisablePlugin() {
    QMessageBox::information(this, "提示", "禁用插件功能待实现");
}
