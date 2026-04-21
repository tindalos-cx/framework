# Plugin Framework Phase 3: Sample Plugins + Integration

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Build two sample plugins (SamplePlugin, CounterPlugin) and verify the full framework works end-to-end — compile, run, load plugins via GUI.

**Architecture:** Two DLL plugins demonstrating the full interface surface: lifecycle, events, services, and UI pages.

**Tech Stack:** C++17, Qt6::Widgets, CMake 3.20+

**Depends on:** Phase 1 (PluginSDK + PluginCore) and Phase 2 (HostApp)

---

## File Structure

```
Plugins/
├── CMakeLists.txt                      # add_subdirectory for each plugin
├── SamplePlugin/
│   ├── CMakeLists.txt                  # SHARED library target
│   ├── SamplePlugin.h                  # IPlugin + IEventHandler implementation
│   └── SamplePlugin.cpp                # Lifecycle + event subscription
└── CounterPlugin/
    ├── CMakeLists.txt                  # SHARED library target
    ├── CounterPlugin.h                 # IPlugin + IPluginService + QWidget
    └── CounterPlugin.cpp               # Service + UI page with increment button
```

---

## Task 1: SamplePlugin

**Files:**
- Create: `Plugins/SamplePlugin/SamplePlugin.h`
- Create: `Plugins/SamplePlugin/SamplePlugin.cpp`
- Create: `Plugins/SamplePlugin/CMakeLists.txt`

- [ ] **Step 1: Write SamplePlugin.h**

```cpp
#pragma once
#include "IPlugin.h"
#include "IEventHandler.h"

class SamplePlugin : public IPlugin, public IEventHandler {
public:
    PluginMetadata GetMetadata() const override;
    bool Initialize(IHost* host) override;
    bool Start() override;
    bool Stop() override;
    void Destroy() override;
    void OnEvent(const std::string& eventType, void* eventData) override;

private:
    IHost* host_ = nullptr;
};
```

- [ ] **Step 2: Write SamplePlugin.cpp**

```cpp
#include "SamplePlugin.h"

extern "C" __declspec(dllexport) IPlugin* CreatePlugin() {
    return new SamplePlugin();
}

extern "C" __declspec(dllexport) void DestroyPlugin(IPlugin* plugin) {
    delete plugin;
}

PluginMetadata SamplePlugin::GetMetadata() const {
    return {
        "com.example.sample",
        "Sample Plugin",
        "1.0.0",
        "Framework Team",
        "A sample plugin demonstrating lifecycle and events",
        "1.0.0",
        {}
    };
}

bool SamplePlugin::Initialize(IHost* host) {
    host_ = host;
    return true;
}

bool SamplePlugin::Start() {
    if (host_) host_->SubscribeEvent("TimerTick", this);
    return true;
}

bool SamplePlugin::Stop() {
    if (host_) host_->UnsubscribeEvent("TimerTick", this);
    return true;
}

void SamplePlugin::Destroy() {
    host_ = nullptr;
}

void SamplePlugin::OnEvent(const std::string& eventType, void* eventData) {
    (void)eventData;
    if (host_) host_->LogInfo("SamplePlugin received: " + eventType);
}
```

- [ ] **Step 3: Write SamplePlugin/CMakeLists.txt**

```cmake
add_library(SamplePlugin SHARED SamplePlugin.cpp)
target_link_libraries(SamplePlugin PRIVATE PluginSDK)
set_target_properties(SamplePlugin PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins
)
```

- [ ] **Step 4: Commit**

```bash
git add Plugins/SamplePlugin/
git commit -m "feat: add SamplePlugin demonstrating lifecycle and events"
```

---

## Task 2: CounterPlugin

**Files:**
- Create: `Plugins/CounterPlugin/CounterPlugin.h`
- Create: `Plugins/CounterPlugin/CounterPlugin.cpp`
- Create: `Plugins/CounterPlugin/CMakeLists.txt`

- [ ] **Step 1: Write CounterPlugin.h**

```cpp
#pragma once
#include "IPlugin.h"
#include "IPluginService.h"
#include <QLabel>
#include <QPushButton>

class ICounterService : public IPluginService {
public:
    virtual int GetCount() const = 0;
    virtual void Increment() = 0;
};

class CounterPlugin : public IPlugin, public ICounterService {
public:
    PluginMetadata GetMetadata() const override;
    bool Initialize(IHost* host) override;
    bool Start() override;
    bool Stop() override;
    void Destroy() override;

    IPluginService* GetService(const std::string& serviceName) override;
    std::string GetServiceName() const override;
    int GetCount() const override;
    void Increment() override;

    std::vector<PluginUIPage> GetUIPages() override;
    QWidget* CreateUIPage(const std::string& pageId, QWidget* parent) override;

private:
    IHost* host_ = nullptr;
    int count_ = 0;
    QLabel* label_ = nullptr;
};
```

- [ ] **Step 2: Write CounterPlugin.cpp**

```cpp
#include "CounterPlugin.h"
#include <QVBoxLayout>

extern "C" __declspec(dllexport) IPlugin* CreatePlugin() {
    return new CounterPlugin();
}

extern "C" __declspec(dllexport) void DestroyPlugin(IPlugin* plugin) {
    delete plugin;
}

PluginMetadata CounterPlugin::GetMetadata() const {
    return {
        "com.example.counter",
        "Counter Plugin",
        "1.0.0",
        "Framework Team",
        "A counter plugin with UI and service",
        "1.0.0",
        {}
    };
}

bool CounterPlugin::Initialize(IHost* host) {
    host_ = host;
    return true;
}

bool CounterPlugin::Start() {
    return true;
}

bool CounterPlugin::Stop() {
    return true;
}

void CounterPlugin::Destroy() {
    host_ = nullptr;
}

IPluginService* CounterPlugin::GetService(const std::string& serviceName) {
    if (serviceName == "CounterService") return this;
    return nullptr;
}

std::string CounterPlugin::GetServiceName() const {
    return "CounterService";
}

int CounterPlugin::GetCount() const {
    return count_;
}

void CounterPlugin::Increment() {
    ++count_;
    if (label_) label_->setText(QString::number(count_));
}

std::vector<PluginUIPage> CounterPlugin::GetUIPages() {
    return {{"main", "Counter", ""}};
}

QWidget* CounterPlugin::CreateUIPage(const std::string& pageId, QWidget* parent) {
    (void)pageId;
    auto* widget = new QWidget(parent);
    auto* layout = new QVBoxLayout(widget);

    label_ = new QLabel("0", widget);
    label_->setAlignment(Qt::AlignCenter);
    auto font = label_->font();
    font.setPointSize(24);
    label_->setFont(font);
    layout->addWidget(label_);

    auto* btn = new QPushButton("Increment", widget);
    QObject::connect(btn, &QPushButton::clicked, [this]() { Increment(); });
    layout->addWidget(btn);

    return widget;
}
```

- [ ] **Step 3: Write CounterPlugin/CMakeLists.txt**

```cmake
add_library(CounterPlugin SHARED CounterPlugin.cpp)
target_link_libraries(CounterPlugin PRIVATE PluginSDK)
set_target_properties(CounterPlugin PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins
)
```

- [ ] **Step 4: Commit**

```bash
git add Plugins/CounterPlugin/
git commit -m "feat: add CounterPlugin with service interface and UI page"
```

---

## Task 3: Plugins Root CMake + Full Build

**Files:**
- Create: `Plugins/CMakeLists.txt`
- Modify: `CMakeLists.txt` (root) — add `add_subdirectory(Plugins)`

- [ ] **Step 1: Write Plugins/CMakeLists.txt**

```cmake
add_subdirectory(SamplePlugin)
add_subdirectory(CounterPlugin)
```

- [ ] **Step 2: Modify root CMakeLists.txt**

Add at the end of `CMakeLists.txt`:

```cmake
add_subdirectory(Plugins)
```

- [ ] **Step 3: Full build**

Run:
```bash
cd build && cmake .. && cmake --build .
```

Expected:
- `PluginCore.lib` built
- `HostApp.exe` built
- `SamplePlugin.dll` built → output to `build/plugins/SamplePlugin.dll`
- `CounterPlugin.dll` built → output to `build/plugins/CounterPlugin.dll`

- [ ] **Step 4: Commit**

```bash
git add Plugins/CMakeLists.txt CMakeLists.txt
git commit -m "build: wire up Plugins directory and verify full build"
```

---

## Task 4: Integration Verification

**Files:** None new. Run the built application.

- [ ] **Step 1: Run HostApp and verify initial state**

Run:
```bash
cd build && ./HostApp
```

Expected: Window opens with title "Plugin Framework Host". Left panel shows "Manage Plugins" button. No plugin pages yet (plugins not enabled in default config).

- [ ] **Step 2: Verify plugin discovery in management dialog**

1. Click "Manage Plugins"
2. Dialog opens showing both plugins:
   - Sample Plugin | 1.0.0 | Discovered | com.example.sample
   - Counter Plugin | 1.0.0 | Discovered | com.example.counter
3. Both have unchecked checkboxes

- [ ] **Step 3: Enable and load CounterPlugin**

1. Check the box for "Counter Plugin"
2. Click "Apply Changes"
3. Status changes to "Running"
4. Click "OK" to close dialog
5. Left navigation now shows "Counter" entry
6. Click "Counter" — content area shows "0" and "Increment" button
7. Click "Increment" — number increases

- [ ] **Step 4: Enable and load SamplePlugin**

1. Click "Manage Plugins" again
2. Check "Sample Plugin"
3. Click "Apply Changes"
4. Status changes to "Running"

(Note: SamplePlugin has no UI page, so no new nav entry appears — this is expected.)

- [ ] **Step 5: Verify unload**

1. Open "Manage Plugins"
2. Uncheck "Counter Plugin"
3. Click "Apply Changes"
4. Status changes to "Unloaded"
5. "Counter" nav entry disappears from main window

- [ ] **Step 6: Verify reload**

1. Check "Counter Plugin" again
2. Click "Apply Changes"
3. Plugin loads fresh (count resets to 0 — expected, state not persisted)

- [ ] **Step 7: Commit completion**

```bash
git add -A
git commit -m "feat: complete Phase 3 — sample plugins and integration verified"
```

---

## Self-Review

### Spec Coverage Check

| Spec Section | Implementing Task |
|--------------|------------------|
| DLL 导出函数约定 (CreatePlugin/DestroyPlugin) | Task 1, 2 |
| SamplePlugin (生命周期 + 事件) | Task 1 |
| CounterPlugin (服务 + UI) | Task 2 |
| 插件构建配置 | Task 1, 2, 3 |
| 集成验证 | Task 4 |

### Placeholder Scan

- No "TBD", "TODO", "implement later" found.
- All `__declspec(dllexport)` decorations present.
- All Qt `QObject::connect` calls use correct syntax.
- `extern "C"` used correctly for DLL exports to prevent name mangling.

### Type Consistency Check

- `PluginMetadata` fields match between SamplePlugin, CounterPlugin, and PluginTypes.h
- `IPlugin` virtual methods match SDK header
- `ICounterService` extends `IPluginService` correctly
- `GetService()` returns `this` cast to `IPluginService*` — valid inheritance

**Phase 3 is complete and ready for execution.**
