# Plugin Framework Phase 2: HostApp GUI Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Build the Qt-based Host Application with plugin management dialog, left navigation panel for plugin pages, and IHost implementation wiring everything together.

**Architecture:** Qt6 Widgets application. MainWindow hosts a QListWidget navigation and QStackedWidget content area. PluginManagerDialog lists plugins with checkboxes for enable/disable. HostImpl bridges Core services to the IHost interface.

**Tech Stack:** C++17, Qt6::Widgets, CMake 3.20+

**Depends on:** Phase 1 (PluginSDK + PluginCore must compile)

---

## File Structure

```
HostApp/
├── CMakeLists.txt                 # Executable target: HostApp
├── HostImpl.h/.cpp                # IHost implementation bridging to Core
├── MainWindow.h/.cpp              # Main window: nav + stacked widget
├── PluginManagerDialog.h/.cpp     # Plugin management dialog
└── main.cpp                       # Entry point: config, scan, load, show UI
```

---

## Task 1: HostImpl (IHost Bridge)

**Files:**
- Create: `HostApp/HostImpl.h`
- Create: `HostApp/HostImpl.cpp`

- [ ] **Step 1: Write HostImpl.h**

```cpp
#pragma once
#include "IHost.h"
#include "PluginManager.h"
#include "ConfigManager.h"

class HostImpl : public IHost {
public:
    HostImpl(ConfigManager* configManager, PluginManager* pluginManager);

    std::string GetConfig(const std::string& key, const std::string& defaultValue) override;
    void SetConfig(const std::string& key, const std::string& value) override;

    void LogInfo(const std::string& msg) override;
    void LogWarn(const std::string& msg) override;
    void LogError(const std::string& msg) override;

    IPluginService* FindService(const std::string& pluginId, const std::string& serviceName) override;

    void PublishEvent(const std::string& eventType, void* eventData) override;
    void SubscribeEvent(const std::string& eventType, IEventHandler* handler) override;
    void UnsubscribeEvent(const std::string& eventType, IEventHandler* handler) override;

private:
    ConfigManager* configManager_;
    PluginManager* pluginManager_;
};
```

- [ ] **Step 2: Write HostImpl.cpp**

```cpp
#include "HostImpl.h"
#include <QDebug>

HostImpl::HostImpl(ConfigManager* configManager, PluginManager* pluginManager)
    : configManager_(configManager), pluginManager_(pluginManager) {}

std::string HostImpl::GetConfig(const std::string& key, const std::string& defaultValue) {
    auto cfg = configManager_->GetPluginConfig("");
    auto it = cfg.find(key);
    if (it == cfg.end()) return defaultValue;
    try {
        return it->get<std::string>();
    } catch (...) {
        return defaultValue;
    }
}

void HostImpl::SetConfig(const std::string& key, const std::string& value) {
    auto cfg = configManager_->GetPluginConfig("");
    cfg[key] = value;
    configManager_->SetPluginConfig("", cfg);
    configManager_->Save();
}

void HostImpl::LogInfo(const std::string& msg) {
    qDebug() << "[INFO]" << QString::fromStdString(msg);
}

void HostImpl::LogWarn(const std::string& msg) {
    qDebug() << "[WARN]" << QString::fromStdString(msg);
}

void HostImpl::LogError(const std::string& msg) {
    qDebug() << "[ERROR]" << QString::fromStdString(msg);
}

IPluginService* HostImpl::FindService(const std::string& pluginId, const std::string& serviceName) {
    return pluginManager_->GetRegistry().Find(pluginId, serviceName);
}

void HostImpl::PublishEvent(const std::string& eventType, void* eventData) {
    pluginManager_->GetEventBus().Publish(eventType, eventData);
}

void HostImpl::SubscribeEvent(const std::string& eventType, IEventHandler* handler) {
    pluginManager_->GetEventBus().Subscribe(eventType, handler);
}

void HostImpl::UnsubscribeEvent(const std::string& eventType, IEventHandler* handler) {
    pluginManager_->GetEventBus().Unsubscribe(eventType, handler);
}
```

- [ ] **Step 3: Commit**

```bash
git add HostApp/HostImpl.h HostApp/HostImpl.cpp
git commit -m "feat: add HostImpl bridging Core to IHost interface"
```

---

## Task 2: MainWindow

**Files:**
- Create: `HostApp/MainWindow.h`
- Create: `HostApp/MainWindow.cpp`

- [ ] **Step 1: Write MainWindow.h**

```cpp
#pragma once
#include <QMainWindow>
#include <QListWidget>
#include <QStackedWidget>
#include "PluginManager.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(PluginManager* pluginManager, QWidget* parent = nullptr);

private slots:
    void onPluginItemClicked(QListWidgetItem* item);
    void onManagePlugins();

private:
    void refreshPluginPages();

    PluginManager* pluginManager_;
    QListWidget* navList_;
    QStackedWidget* stackedWidget_;
};
```

- [ ] **Step 2: Write MainWindow.cpp**

```cpp
#include "MainWindow.h"
#include "PluginManagerDialog.h"
#include "IPlugin.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>

MainWindow::MainWindow(PluginManager* pluginManager, QWidget* parent)
    : QMainWindow(parent), pluginManager_(pluginManager) {

    auto* central = new QWidget(this);
    auto* hLayout = new QHBoxLayout(central);

    auto* leftPanel = new QWidget(this);
    auto* vLayout = new QVBoxLayout(leftPanel);

    navList_ = new QListWidget(this);
    vLayout->addWidget(navList_);

    auto* manageBtn = new QPushButton("Manage Plugins", this);
    connect(manageBtn, &QPushButton::clicked, this, &MainWindow::onManagePlugins);
    vLayout->addWidget(manageBtn);

    hLayout->addWidget(leftPanel, 1);
    stackedWidget_ = new QStackedWidget(this);
    hLayout->addWidget(stackedWidget_, 4);

    setCentralWidget(central);
    setWindowTitle("Plugin Framework Host");

    connect(navList_, &QListWidget::itemClicked, this, &MainWindow::onPluginItemClicked);

    refreshPluginPages();
}

void MainWindow::refreshPluginPages() {
    navList_->clear();
    while (stackedWidget_->count() > 0) {
        auto* w = stackedWidget_->widget(0);
        stackedWidget_->removeWidget(w);
        delete w;
    }

    for (auto& info : pluginManager_->GetPlugins()) {
        if (info.state != PluginState::Running) continue;
        if (!info.instance) continue;
        auto pages = info.instance->GetUIPages();
        for (auto& page : pages) {
            auto* widget = info.instance->CreateUIPage(page.pageId, stackedWidget_);
            if (widget) {
                stackedWidget_->addWidget(widget);
                auto* item = new QListWidgetItem(QString::fromStdString(page.title), navList_);
                item->setData(Qt::UserRole, stackedWidget_->count() - 1);
            }
        }
    }
}

void MainWindow::onPluginItemClicked(QListWidgetItem* item) {
    int index = item->data(Qt::UserRole).toInt();
    stackedWidget_->setCurrentIndex(index);
}

void MainWindow::onManagePlugins() {
    PluginManagerDialog dlg(pluginManager_, this);
    if (dlg.exec() == QDialog::Accepted) {
        refreshPluginPages();
    }
}
```

- [ ] **Step 3: Commit**

```bash
git add HostApp/MainWindow.h HostApp/MainWindow.cpp
git commit -m "feat: add MainWindow with plugin page navigation"
```

---

## Task 3: PluginManagerDialog

**Files:**
- Create: `HostApp/PluginManagerDialog.h`
- Create: `HostApp/PluginManagerDialog.cpp`

- [ ] **Step 1: Write PluginManagerDialog.h**

```cpp
#pragma once
#include <QDialog>
#include <QTableWidget>
#include "PluginManager.h"

class PluginManagerDialog : public QDialog {
    Q_OBJECT
public:
    explicit PluginManagerDialog(PluginManager* pluginManager, QWidget* parent = nullptr);

private slots:
    void onApply();
    void onRefresh();

private:
    void populateTable();

    PluginManager* pluginManager_;
    QTableWidget* table_;
};
```

- [ ] **Step 2: Write PluginManagerDialog.cpp**

```cpp
#include "PluginManagerDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QHeaderView>
#include <QCheckBox>
#include <QMessageBox>

PluginManagerDialog::PluginManagerDialog(PluginManager* pluginManager, QWidget* parent)
    : QDialog(parent), pluginManager_(pluginManager) {

    setWindowTitle("Plugin Manager");
    setMinimumSize(700, 400);

    auto* vLayout = new QVBoxLayout(this);

    table_ = new QTableWidget(this);
    table_->setColumnCount(5);
    table_->setHorizontalHeaderLabels({"Enable", "Name", "Version", "Status", "ID"});
    table_->horizontalHeader()->setStretchLastSection(true);
    table_->setSelectionBehavior(QAbstractItemView::SelectRows);
    vLayout->addWidget(table_);

    auto* hLayout = new QHBoxLayout();
    auto* refreshBtn = new QPushButton("Refresh", this);
    auto* applyBtn = new QPushButton("Apply Changes", this);
    hLayout->addWidget(refreshBtn);
    hLayout->addWidget(applyBtn);
    hLayout->addStretch();
    vLayout->addLayout(hLayout);

    connect(refreshBtn, &QPushButton::clicked, this, &PluginManagerDialog::onRefresh);
    connect(applyBtn, &QPushButton::clicked, this, &PluginManagerDialog::onApply);

    populateTable();
}

void PluginManagerDialog::populateTable() {
    table_->setRowCount(0);
    for (auto& info : pluginManager_->GetPlugins()) {
        int row = table_->rowCount();
        table_->insertRow(row);

        auto* check = new QCheckBox();
        check->setChecked(info.enabled);
        table_->setCellWidget(row, 0, check);

        table_->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(info.metadata.name)));
        table_->setItem(row, 2, new QTableWidgetItem(QString::fromStdString(info.metadata.version)));

        QString status;
        switch (info.state) {
            case PluginState::Running: status = "Running"; break;
            case PluginState::Ready: status = "Ready"; break;
            case PluginState::Error: status = "Error: " + QString::fromStdString(info.errorMessage); break;
            case PluginState::Discovered: status = "Discovered"; break;
            case PluginState::Unloaded: status = "Unloaded"; break;
            default: status = "Unknown"; break;
        }
        table_->setItem(row, 3, new QTableWidgetItem(status));
        table_->setItem(row, 4, new QTableWidgetItem(QString::fromStdString(info.metadata.id)));
    }
}

void PluginManagerDialog::onApply() {
    for (int row = 0; row < table_->rowCount(); ++row) {
        auto* check = qobject_cast<QCheckBox*>(table_->cellWidget(row, 0));
        if (!check) continue;
        QString id = table_->item(row, 4)->text();
        bool enabled = check->isChecked();
        auto* info = pluginManager_->FindPlugin(id.toStdString());
        if (!info) continue;

        if (enabled && info->state != PluginState::Running) {
            if (!pluginManager_->LoadPlugin(id.toStdString())) {
                QMessageBox::warning(this, "Load Failed",
                    QString("Failed to load %1: %2")
                        .arg(id)
                        .arg(QString::fromStdString(info->errorMessage)));
            }
        } else if (!enabled && info->state == PluginState::Running) {
            pluginManager_->UnloadPlugin(id.toStdString());
        }
    }
    populateTable();
}

void PluginManagerDialog::onRefresh() {
    pluginManager_->Scan();
    populateTable();
}
```

- [ ] **Step 3: Commit**

```bash
git add HostApp/PluginManagerDialog.h HostApp/PluginManagerDialog.cpp
git commit -m "feat: add PluginManagerDialog for enable/disable/reload"
```

---

## Task 4: main.cpp + HostApp CMakeLists.txt

**Files:**
- Create: `HostApp/main.cpp`
- Create: `HostApp/CMakeLists.txt`
- Modify: `CMakeLists.txt` (root) — add `add_subdirectory(HostApp)`

- [ ] **Step 1: Write HostApp/main.cpp**

```cpp
#include <QApplication>
#include "MainWindow.h"
#include "PluginManager.h"
#include "ConfigManager.h"
#include "HostImpl.h"
#include <QDir>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    ConfigManager configManager("plugins.json");
    configManager.Load();

    PluginManager pluginManager(&configManager);
    pluginManager.Scan();

    HostImpl host(&configManager, &pluginManager);
    pluginManager.Initialize(&host);

    // Auto-load enabled plugins
    for (auto& info : pluginManager.GetPlugins()) {
        if (info.enabled) {
            pluginManager.LoadPlugin(info.metadata.id);
        }
    }

    MainWindow window(&pluginManager);
    window.show();

    return app.exec();
}
```

- [ ] **Step 2: Write HostApp/CMakeLists.txt**

```cmake
add_executable(HostApp
    main.cpp
    MainWindow.cpp
    PluginManagerDialog.cpp
    HostImpl.cpp
)
target_include_directories(HostApp PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(HostApp PRIVATE PluginCore Qt6::Widgets)
```

- [ ] **Step 3: Modify root CMakeLists.txt to add HostApp**

In `CMakeLists.txt`, add after `add_subdirectory(Core)`:

```cmake
add_subdirectory(HostApp)
```

- [ ] **Step 4: Build HostApp**

Run:
```bash
cd build && cmake .. && cmake --build . --target HostApp
```

Expected: Compiles without errors. `HostApp.exe` generated.

- [ ] **Step 5: Commit**

```bash
git add HostApp/ CMakeLists.txt
git commit -m "feat: add HostApp with Qt GUI and IHost wiring"
```

---

## Self-Review

### Spec Coverage Check

| Spec Section | Implementing Task |
|--------------|------------------|
| IHost 实现 | Task 1 (HostImpl) |
| 宿主主窗口 | Task 2 (MainWindow) |
| 插件管理对话框 | Task 3 (PluginManagerDialog) |
| 插件 UI 集成 (QWidget*) | Task 2 (refreshPluginPages) |
| 入口点 / 自动加载 | Task 4 (main.cpp) |
| HostApp 构建 | Task 4 (CMakeLists.txt) |

### Placeholder Scan

- No "TBD", "TODO", "fill in later" found.
- All Qt signal/slot connections use correct macro syntax (`Q_OBJECT`, `slots`, `signals`).
- All code blocks contain complete implementations.

### Type Consistency Check

- `PluginState` enum used in MainWindow and PluginManagerDialog matches PluginTypes.h definition
- `IHost` method signatures in HostImpl match PluginSDK/IHost.h
- `PluginManager*` pointer types consistent across MainWindow, PluginManagerDialog, HostImpl

**Phase 2 is complete and ready for execution.**
