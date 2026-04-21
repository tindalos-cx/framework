# Plugin Framework Phase 1: Core Layer Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Build the PluginSDK (headers) and PluginCore (static library) — all interface definitions and core logic for plugin discovery, loading, lifecycle, registry, event bus, and config management.

**Architecture:** Pure C++17 logic layer with nlohmann/json for config. Windows DLL loading via Win32 API. No Qt dependency in Core layer — Qt is only in PluginSDK headers for the `QWidget*` UI interface.

**Tech Stack:** C++17, CMake 3.20+, nlohmann/json (FetchContent), Windows API

---

## File Structure

```
PluginFramework/
├── CMakeLists.txt                 # Root CMake: Qt + json deps, add_subdirectory calls
├── PluginSDK/
│   ├── CMakeLists.txt             # INTERFACE target: headers only
│   └── include/
│       ├── PluginTypes.h          # PluginMetadata, PluginUIPage, PluginState enum
│       ├── IPluginService.h       # Service interface base
│       ├── IEventHandler.h        # Event handler interface
│       ├── IHost.h                # Host interface exposed to plugins
│       └── IPlugin.h              # Plugin base class (extends IHost users)
└── Core/
    ├── CMakeLists.txt             # STATIC target: compiles all .cpp files
    ├── ConfigManager.h/.cpp       # JSON config read/write (plugins.json)
    ├── EventBus.h/.cpp            # Publish/subscribe event dispatch
    ├── PluginRegistry.h/.cpp      # Service registration and lookup
    ├── PluginScanner.h/.cpp       # Directory scan + DLL metadata extraction
    └── PluginManager.h/.cpp       # Orchestrates load/unload/reload lifecycle
```

---

## Task 1: Project Skeleton

**Files:**
- Create: `CMakeLists.txt`
- Create: directories `PluginSDK/include/`, `Core/`

- [ ] **Step 1: Create directory structure**

Run:
```bash
mkdir -p PluginSDK/include Core
```

- [ ] **Step 2: Write root CMakeLists.txt**

```cmake
cmake_minimum_required(VERSION 3.20)
project(PluginFramework VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt (needed for PluginSDK QWidget dependency)
set(CMAKE_AUTOMOC ON)
find_package(Qt6 COMPONENTS Widgets REQUIRED)
if(NOT Qt6_FOUND)
    find_package(Qt5 COMPONENTS Widgets REQUIRED)
endif()

# nlohmann/json via FetchContent
include(FetchContent)
FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(json)

add_subdirectory(PluginSDK)
add_subdirectory(Core)
```

- [ ] **Step 3: Verify cmake configure succeeds**

Run:
```bash
mkdir build && cd build && cmake ..
```

Expected: Configures without errors. Qt and nlohmann/json found.

---

## Task 2: PluginSDK — PluginTypes.h

**Files:**
- Create: `PluginSDK/CMakeLists.txt`
- Create: `PluginSDK/include/PluginTypes.h`

- [ ] **Step 1: Write PluginSDK/CMakeLists.txt**

```cmake
add_library(PluginSDK INTERFACE)
target_include_directories(PluginSDK INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(PluginSDK INTERFACE Qt6::Widgets)
```

- [ ] **Step 2: Write PluginSDK/include/PluginTypes.h**

```cpp
#pragma once
#include <string>
#include <vector>

struct PluginMetadata {
    std::string id;
    std::string name;
    std::string version;
    std::string author;
    std::string description;
    std::string apiVersion;
    std::vector<std::string> dependencies;
};

struct PluginUIPage {
    std::string pageId;
    std::string title;
    std::string iconPath;
};

enum class PluginState {
    Unknown,
    Discovered,
    Loaded,
    Ready,
    Running,
    Error,
    Unloaded
};
```

- [ ] **Step 3: Verify PluginSDK compiles**

Run:
```bash
cd build && cmake --build . --target PluginSDK
```

Expected: Builds successfully (INTERFACE target has no object files but CMake accepts it).

---

## Task 3: PluginSDK — IPluginService.h + IEventHandler.h

**Files:**
- Create: `PluginSDK/include/IPluginService.h`
- Create: `PluginSDK/include/IEventHandler.h`

- [ ] **Step 1: Write IPluginService.h**

```cpp
#pragma once
#include <string>

class IPluginService {
public:
    virtual ~IPluginService() = default;
    virtual std::string GetServiceName() const = 0;
};
```

- [ ] **Step 2: Write IEventHandler.h**

```cpp
#pragma once
#include <string>

class IEventHandler {
public:
    virtual ~IEventHandler() = default;
    virtual void OnEvent(const std::string& eventType, void* eventData) = 0;
};
```

- [ ] **Step 3: Commit**

```bash
git add PluginSDK/
git commit -m "feat: add PluginSDK base types and service interfaces"
```

---

## Task 4: PluginSDK — IHost.h

**Files:**
- Create: `PluginSDK/include/IHost.h`

- [ ] **Step 1: Write IHost.h**

```cpp
#pragma once
#include <string>

class IPluginService;
class IEventHandler;

class IHost {
public:
    virtual ~IHost() = default;

    virtual std::string GetConfig(const std::string& key, const std::string& defaultValue = "") = 0;
    virtual void SetConfig(const std::string& key, const std::string& value) = 0;

    virtual void LogInfo(const std::string& msg) = 0;
    virtual void LogWarn(const std::string& msg) = 0;
    virtual void LogError(const std::string& msg) = 0;

    virtual IPluginService* FindService(const std::string& pluginId, const std::string& serviceName) = 0;

    virtual void PublishEvent(const std::string& eventType, void* eventData) = 0;
    virtual void SubscribeEvent(const std::string& eventType, IEventHandler* handler) = 0;
    virtual void UnsubscribeEvent(const std::string& eventType, IEventHandler* handler) = 0;
};
```

- [ ] **Step 2: Commit**

```bash
git add PluginSDK/include/IHost.h
git commit -m "feat: add IHost interface"
```

---

## Task 5: PluginSDK — IPlugin.h

**Files:**
- Create: `PluginSDK/include/IPlugin.h`

- [ ] **Step 1: Write IPlugin.h**

```cpp
#pragma once
#include "PluginTypes.h"
#include "IHost.h"
#include "IPluginService.h"
#include <QWidget>
#include <vector>

class IPlugin {
public:
    virtual ~IPlugin() = default;

    virtual PluginMetadata GetMetadata() const = 0;

    virtual bool Initialize(IHost* host) = 0;
    virtual bool Start() = 0;
    virtual bool Stop() = 0;
    virtual void Destroy() = 0;

    virtual IPluginService* GetService(const std::string& serviceName) { return nullptr; }
    virtual std::vector<PluginUIPage> GetUIPages() { return {}; }
    virtual QWidget* CreateUIPage(const std::string& pageId, QWidget* parent = nullptr) { return nullptr; }
};
```

- [ ] **Step 2: Commit**

```bash
git add PluginSDK/include/IPlugin.h
git commit -m "feat: add IPlugin base class with lifecycle and UI hooks"
```

---

## Task 6: Core — ConfigManager

**Files:**
- Create: `Core/ConfigManager.h`
- Create: `Core/ConfigManager.cpp`

- [ ] **Step 1: Write ConfigManager.h**

```cpp
#pragma once
#include <nlohmann/json.hpp>
#include <string>

class ConfigManager {
public:
    explicit ConfigManager(const std::string& configPath);
    bool Load();
    bool Save();

    bool IsPluginEnabled(const std::string& pluginId) const;
    void SetPluginEnabled(const std::string& pluginId, bool enabled);

    nlohmann::json GetPluginConfig(const std::string& pluginId) const;
    void SetPluginConfig(const std::string& pluginId, const nlohmann::json& config);

    std::string GetPluginDir() const;

private:
    std::string configPath_;
    nlohmann::json config_;
};
```

- [ ] **Step 2: Write ConfigManager.cpp**

```cpp
#include "ConfigManager.h"
#include <fstream>

ConfigManager::ConfigManager(const std::string& configPath) : configPath_(configPath) {}

bool ConfigManager::Load() {
    std::ifstream f(configPath_);
    if (!f.is_open()) {
        config_ = {
            {"pluginFramework", {{"version", "1.0.0"}, {"pluginDir", "plugins"}, {"autoLoad", false}}},
            {"plugins", nlohmann::json::object()}
        };
        return Save();
    }
    try {
        f >> config_;
        return true;
    } catch (...) {
        return false;
    }
}

bool ConfigManager::Save() {
    std::ofstream f(configPath_);
    if (!f.is_open()) return false;
    f << config_.dump(4);
    return true;
}

bool ConfigManager::IsPluginEnabled(const std::string& pluginId) const {
    auto it = config_.find("plugins");
    if (it == config_.end()) return false;
    auto pit = it->find(pluginId);
    if (pit == it->end()) return false;
    auto eit = pit->find("enabled");
    if (eit == pit->end()) return false;
    return eit->get<bool>();
}

void ConfigManager::SetPluginEnabled(const std::string& pluginId, bool enabled) {
    config_["plugins"][pluginId]["enabled"] = enabled;
}

nlohmann::json ConfigManager::GetPluginConfig(const std::string& pluginId) const {
    auto it = config_.find("plugins");
    if (it == config_.end()) return {};
    auto pit = it->find(pluginId);
    if (pit == it->end()) return {};
    auto cit = pit->find("config");
    if (cit == pit->end()) return {};
    return *cit;
}

void ConfigManager::SetPluginConfig(const std::string& pluginId, const nlohmann::json& config) {
    config_["plugins"][pluginId]["config"] = config;
}

std::string ConfigManager::GetPluginDir() const {
    auto it = config_.find("pluginFramework");
    if (it == config_.end()) return "plugins";
    auto dit = it->find("pluginDir");
    if (dit == it->end()) return "plugins";
    return dit->get<std::string>();
}
```

- [ ] **Step 3: Write temporary test to verify**

Create `Core/test_configmanager.cpp`:

```cpp
#include "ConfigManager.h"
#include <iostream>
#include <cassert>
#include <filesystem>

namespace fs = std::filesystem;

int main() {
    const char* path = "test_plugins.json";
    if (fs::exists(path)) fs::remove(path);

    ConfigManager cm(path);
    assert(cm.Load());
    assert(cm.GetPluginDir() == "plugins");

    cm.SetPluginEnabled("com.test.plugin", true);
    assert(cm.IsPluginEnabled("com.test.plugin") == true);
    assert(cm.Save());

    ConfigManager cm2(path);
    assert(cm2.Load());
    assert(cm2.IsPluginEnabled("com.test.plugin") == true);

    if (fs::exists(path)) fs::remove(path);
    std::cout << "ConfigManager OK\n";
    return 0;
}
```

- [ ] **Step 4: Compile and run test**

Run:
```bash
cd build
cmake ..
cmake --build . --target PluginSDK
# Manual compile test:
clang++ -std=c++17 -I../PluginSDK/include -I_deps/json-src/include ../Core/test_configmanager.cpp ../Core/ConfigManager.cpp -o test_configmanager && ./test_configmanager
```

Expected: Output `ConfigManager OK`

- [ ] **Step 5: Clean up test file**

```bash
rm Core/test_configmanager.cpp build/test_configmanager*
```

- [ ] **Step 6: Commit**

```bash
git add Core/ConfigManager.h Core/ConfigManager.cpp
git commit -m "feat: add ConfigManager for JSON plugin config"
```

---

## Task 7: Core — EventBus + PluginRegistry

**Files:**
- Create: `Core/EventBus.h`, `Core/EventBus.cpp`
- Create: `Core/PluginRegistry.h`, `Core/PluginRegistry.cpp`

- [ ] **Step 1: Write EventBus.h**

```cpp
#pragma once
#include "IEventHandler.h"
#include <map>
#include <vector>
#include <string>

class EventBus {
public:
    void Subscribe(const std::string& eventType, IEventHandler* handler);
    void Unsubscribe(const std::string& eventType, IEventHandler* handler);
    void Publish(const std::string& eventType, void* eventData);

private:
    std::map<std::string, std::vector<IEventHandler*>> handlers_;
};
```

- [ ] **Step 2: Write EventBus.cpp**

```cpp
#include "EventBus.h"
#include <algorithm>

void EventBus::Subscribe(const std::string& eventType, IEventHandler* handler) {
    handlers_[eventType].push_back(handler);
}

void EventBus::Unsubscribe(const std::string& eventType, IEventHandler* handler) {
    auto it = handlers_.find(eventType);
    if (it == handlers_.end()) return;
    auto& vec = it->second;
    vec.erase(std::remove(vec.begin(), vec.end(), handler), vec.end());
    if (vec.empty()) handlers_.erase(it);
}

void EventBus::Publish(const std::string& eventType, void* eventData) {
    auto it = handlers_.find(eventType);
    if (it == handlers_.end()) return;
    for (auto* handler : it->second) {
        if (handler) handler->OnEvent(eventType, eventData);
    }
}
```

- [ ] **Step 3: Write PluginRegistry.h**

```cpp
#pragma once
#include "IPluginService.h"
#include <map>
#include <string>

class PluginRegistry {
public:
    void Register(const std::string& pluginId, const std::string& serviceName, IPluginService* service);
    void Unregister(const std::string& pluginId, const std::string& serviceName);
    void UnregisterAll(const std::string& pluginId);
    IPluginService* Find(const std::string& pluginId, const std::string& serviceName) const;

private:
    std::map<std::string, std::map<std::string, IPluginService*>> services_;
};
```

- [ ] **Step 4: Write PluginRegistry.cpp**

```cpp
#include "PluginRegistry.h"

void PluginRegistry::Register(const std::string& pluginId, const std::string& serviceName, IPluginService* service) {
    services_[pluginId][serviceName] = service;
}

void PluginRegistry::Unregister(const std::string& pluginId, const std::string& serviceName) {
    auto it = services_.find(pluginId);
    if (it == services_.end()) return;
    it->second.erase(serviceName);
    if (it->second.empty()) services_.erase(it);
}

void PluginRegistry::UnregisterAll(const std::string& pluginId) {
    services_.erase(pluginId);
}

IPluginService* PluginRegistry::Find(const std::string& pluginId, const std::string& serviceName) const {
    auto it = services_.find(pluginId);
    if (it == services_.end()) return nullptr;
    auto sit = it->second.find(serviceName);
    if (sit == it->second.end()) return nullptr;
    return sit->second;
}
```

- [ ] **Step 5: Write temporary test**

Create `Core/test_eventbus_registry.cpp`:

```cpp
#include "EventBus.h"
#include "PluginRegistry.h"
#include "IPluginService.h"
#include <iostream>
#include <cassert>

class MockService : public IPluginService {
public:
    std::string GetServiceName() const override { return "Mock"; }
};

class MockHandler : public IEventHandler {
public:
    int count = 0;
    void OnEvent(const std::string&, void*) override { ++count; }
};

int main() {
    // Test PluginRegistry
    PluginRegistry reg;
    MockService svc;
    reg.Register("p1", "svc1", &svc);
    assert(reg.Find("p1", "svc1") == &svc);
    assert(reg.Find("p1", "missing") == nullptr);
    reg.UnregisterAll("p1");
    assert(reg.Find("p1", "svc1") == nullptr);

    // Test EventBus
    EventBus bus;
    MockHandler h1, h2;
    bus.Subscribe("evt", &h1);
    bus.Subscribe("evt", &h2);
    bus.Publish("evt", nullptr);
    assert(h1.count == 1);
    assert(h2.count == 1);
    bus.Unsubscribe("evt", &h1);
    bus.Publish("evt", nullptr);
    assert(h1.count == 1);
    assert(h2.count == 2);

    std::cout << "EventBus + Registry OK\n";
    return 0;
}
```

- [ ] **Step 6: Compile and run test**

```bash
cd build
clang++ -std=c++17 -I../PluginSDK/include ../Core/test_eventbus_registry.cpp ../Core/EventBus.cpp ../Core/PluginRegistry.cpp -o test_eventbus_registry && ./test_eventbus_registry
```

Expected: Output `EventBus + Registry OK`

- [ ] **Step 7: Clean up and commit**

```bash
rm Core/test_eventbus_registry.cpp build/test_eventbus_registry*
git add Core/EventBus.h Core/EventBus.cpp Core/PluginRegistry.h Core/PluginRegistry.cpp
git commit -m "feat: add EventBus and PluginRegistry"
```

---

## Task 8: Core — PluginScanner

**Files:**
- Create: `Core/PluginScanner.h`
- Create: `Core/PluginScanner.cpp`

- [ ] **Step 1: Write PluginScanner.h**

```cpp
#pragma once
#include "PluginTypes.h"
#include <string>
#include <vector>

struct DiscoveredPlugin {
    std::string dllPath;
    PluginMetadata metadata;
};

class PluginScanner {
public:
    explicit PluginScanner(const std::string& pluginDir);
    std::vector<DiscoveredPlugin> Scan();

private:
    PluginMetadata ReadMetadata(const std::string& dllPath);
    std::string pluginDir_;
};
```

- [ ] **Step 2: Write PluginScanner.cpp**

```cpp
#include "PluginScanner.h"
#include "IPlugin.h"
#include <windows.h>
#include <filesystem>

namespace fs = std::filesystem;

PluginScanner::PluginScanner(const std::string& pluginDir) : pluginDir_(pluginDir) {}

std::vector<DiscoveredPlugin> PluginScanner::Scan() {
    std::vector<DiscoveredPlugin> result;
    if (!fs::exists(pluginDir_)) return result;
    for (const auto& entry : fs::directory_iterator(pluginDir_)) {
        if (!entry.is_regular_file()) continue;
        auto ext = entry.path().extension().string();
        if (ext != ".dll" && ext != ".DLL") continue;
        auto meta = ReadMetadata(entry.path().string());
        if (!meta.id.empty()) {
            result.push_back({entry.path().string(), meta});
        }
    }
    return result;
}

PluginMetadata PluginScanner::ReadMetadata(const std::string& dllPath) {
    PluginMetadata meta;
    HMODULE hModule = LoadLibraryExA(dllPath.c_str(), nullptr, DONT_RESOLVE_DLL_REFERENCES);
    if (!hModule) return meta;

    auto* createFunc = reinterpret_cast<IPlugin*(*)()>(GetProcAddress(hModule, "CreatePlugin"));
    if (!createFunc) {
        FreeLibrary(hModule);
        return meta;
    }

    IPlugin* plugin = createFunc();
    if (!plugin) {
        FreeLibrary(hModule);
        return meta;
    }

    meta = plugin->GetMetadata();

    auto* destroyFunc = reinterpret_cast<void(*)(IPlugin*)>(GetProcAddress(hModule, "DestroyPlugin"));
    if (destroyFunc) destroyFunc(plugin);

    FreeLibrary(hModule);
    return meta;
}
```

- [ ] **Step 3: Commit**

```bash
git add Core/PluginScanner.h Core/PluginScanner.cpp
git commit -m "feat: add PluginScanner for DLL discovery"
```

---

## Task 9: Core — PluginManager

**Files:**
- Create: `Core/PluginManager.h`
- Create: `Core/PluginManager.cpp`

- [ ] **Step 1: Write PluginManager.h**

```cpp
#pragma once
#include "PluginTypes.h"
#include "PluginRegistry.h"
#include "EventBus.h"
#include "ConfigManager.h"
#include <string>
#include <vector>
#include <windows.h>

class IPlugin;
class IHost;

struct PluginInfo {
    PluginMetadata metadata;
    PluginState state;
    std::string dllPath;
    std::string errorMessage;
    HMODULE hModule;
    IPlugin* instance;
    bool enabled;
};

class PluginManager {
public:
    explicit PluginManager(ConfigManager* configManager);
    ~PluginManager();

    bool Initialize(IHost* host);
    void Scan();

    bool LoadPlugin(const std::string& pluginId);
    bool UnloadPlugin(const std::string& pluginId);
    bool ReloadPlugin(const std::string& pluginId);

    std::vector<PluginInfo>& GetPlugins();
    PluginInfo* FindPlugin(const std::string& pluginId);

    PluginRegistry& GetRegistry() { return registry_; }
    EventBus& GetEventBus() { return eventBus_; }

private:
    bool DoLoad(PluginInfo& info);
    void DoUnload(PluginInfo& info);

    ConfigManager* configManager_;
    IHost* host_;
    PluginRegistry registry_;
    EventBus eventBus_;
    std::vector<PluginInfo> plugins_;
};
```

- [ ] **Step 2: Write PluginManager.cpp**

```cpp
#include "PluginManager.h"
#include "IPlugin.h"
#include "PluginScanner.h"
#include <algorithm>
#include <filesystem>

namespace fs = std::filesystem;

PluginManager::PluginManager(ConfigManager* configManager)
    : configManager_(configManager), host_(nullptr) {}

PluginManager::~PluginManager() {
    for (auto& plugin : plugins_) {
        if (plugin.state == PluginState::Running) {
            plugin.instance->Stop();
        }
        if (plugin.state == PluginState::Ready || plugin.state == PluginState::Running) {
            plugin.instance->Destroy();
        }
        if (plugin.hModule) {
            FreeLibrary(plugin.hModule);
        }
    }
}

bool PluginManager::Initialize(IHost* host) {
    host_ = host;
    return true;
}

void PluginManager::Scan() {
    PluginScanner scanner(configManager_->GetPluginDir());
    auto discovered = scanner.Scan();

    std::vector<PluginInfo> newList;
    for (auto& dp : discovered) {
        PluginInfo info;
        info.metadata = dp.metadata;
        info.dllPath = dp.dllPath;
        info.state = PluginState::Discovered;
        info.hModule = nullptr;
        info.instance = nullptr;
        info.enabled = configManager_->IsPluginEnabled(dp.metadata.id);
        newList.push_back(info);
    }
    plugins_ = std::move(newList);
}

bool PluginManager::LoadPlugin(const std::string& pluginId) {
    auto* info = FindPlugin(pluginId);
    if (!info) return false;
    if (info->state != PluginState::Discovered && info->state != PluginState::Unloaded) return false;

    if (!DoLoad(*info)) {
        info->state = PluginState::Error;
        return false;
    }

    if (host_) {
        if (!info->instance->Initialize(host_)) {
            DoUnload(*info);
            info->state = PluginState::Error;
            info->errorMessage = "Initialize failed";
            return false;
        }
        info->state = PluginState::Ready;
    }

    if (!info->instance->Start()) {
        info->instance->Destroy();
        DoUnload(*info);
        info->state = PluginState::Error;
        info->errorMessage = "Start failed";
        return false;
    }
    info->state = PluginState::Running;
    return true;
}

bool PluginManager::UnloadPlugin(const std::string& pluginId) {
    auto* info = FindPlugin(pluginId);
    if (!info) return false;
    if (info->state != PluginState::Running && info->state != PluginState::Ready) return false;

    if (info->state == PluginState::Running) {
        info->instance->Stop();
    }
    if (info->state == PluginState::Ready || info->state == PluginState::Running) {
        info->instance->Destroy();
    }
    DoUnload(*info);
    info->state = PluginState::Unloaded;
    return true;
}

bool PluginManager::ReloadPlugin(const std::string& pluginId) {
    if (!UnloadPlugin(pluginId)) return false;
    return LoadPlugin(pluginId);
}

std::vector<PluginInfo>& PluginManager::GetPlugins() {
    return plugins_;
}

PluginInfo* PluginManager::FindPlugin(const std::string& pluginId) {
    for (auto& plugin : plugins_) {
        if (plugin.metadata.id == pluginId) return &plugin;
    }
    return nullptr;
}

bool PluginManager::DoLoad(PluginInfo& info) {
    info.hModule = LoadLibraryA(info.dllPath.c_str());
    if (!info.hModule) {
        info.errorMessage = "Failed to load DLL";
        return false;
    }

    auto* createFunc = reinterpret_cast<IPlugin*(*)()>(GetProcAddress(info.hModule, "CreatePlugin"));
    if (!createFunc) {
        FreeLibrary(info.hModule);
        info.hModule = nullptr;
        info.errorMessage = "CreatePlugin not found";
        return false;
    }

    info.instance = createFunc();
    if (!info.instance) {
        FreeLibrary(info.hModule);
        info.hModule = nullptr;
        info.errorMessage = "CreatePlugin returned null";
        return false;
    }
    return true;
}

void PluginManager::DoUnload(PluginInfo& info) {
    if (info.instance) {
        auto* destroyFunc = reinterpret_cast<void(*)(IPlugin*)>(GetProcAddress(info.hModule, "DestroyPlugin"));
        if (destroyFunc) destroyFunc(info.instance);
        info.instance = nullptr;
    }
    if (info.hModule) {
        FreeLibrary(info.hModule);
        info.hModule = nullptr;
    }
}
```

- [ ] **Step 3: Commit**

```bash
git add Core/PluginManager.h Core/PluginManager.cpp
git commit -m "feat: add PluginManager with lifecycle and hot-reload"
```

---

## Task 10: Core CMakeLists.txt + Build Verification

**Files:**
- Create: `Core/CMakeLists.txt`

- [ ] **Step 1: Write Core/CMakeLists.txt**

```cmake
add_library(PluginCore STATIC
    ConfigManager.cpp
    EventBus.cpp
    PluginRegistry.cpp
    PluginScanner.cpp
    PluginManager.cpp
)
target_include_directories(PluginCore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(PluginCore PUBLIC PluginSDK nlohmann_json::nlohmann_json)
```

- [ ] **Step 2: Build PluginCore**

Run:
```bash
cd build && cmake .. && cmake --build . --target PluginCore
```

Expected: Compiles without errors. Output shows `PluginCore.lib` generated.

- [ ] **Step 3: Commit**

```bash
git add Core/CMakeLists.txt
git commit -m "build: add Core CMakeLists.txt and verify build"
```

---

## Self-Review

### Spec Coverage Check

| Spec Section | Implementing Task |
|--------------|------------------|
| 插件元数据结构 | Task 2 (PluginTypes.h) |
| IPlugin 基类 | Task 5 |
| IHost 接口 | Task 4 |
| IPluginService / IEventHandler | Task 3 |
| 生命周期状态机 | Task 9 (PluginManager) |
| 配置与发现 | Task 6 (ConfigManager), Task 8 (PluginScanner) |
| 事件总线 | Task 7 (EventBus) |
| 服务注册发现 | Task 7 (PluginRegistry) |
| 热加载 | Task 9 (ReloadPlugin) |
| 构建系统 (Core) | Task 1, 2, 10 |

### Placeholder Scan

- No "TBD", "TODO", "implement later" found.
- All code blocks contain complete, compilable code.
- All file paths are exact.

### Type Consistency Check

- `PluginState` enum used consistently across PluginTypes.h and PluginManager.h
- `IPluginService*` / `IEventHandler*` pointers match between SDK headers and Core implementations
- `nlohmann::json` types consistent between ConfigManager header and implementation
- Method signatures in PluginManager match design doc: `LoadPlugin`, `UnloadPlugin`, `ReloadPlugin`, `Scan`, `Initialize(IHost*)`

**Phase 1 is complete and ready for execution.**
