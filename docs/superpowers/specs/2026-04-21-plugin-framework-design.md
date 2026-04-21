# C++ DLL Plugin Framework 设计文档

日期：2026-04-21
状态：待实现

---

## 1. 项目概述

一个基于进程内 DLL 的 C++ 通用桌面应用插件框架。核心能力：

- 宿主应用通过加载 DLL 动态扩展功能
- 插件遵循统一的 `IPlugin` 基类接口
- 配置驱动的发现机制：扫描目录 + JSON 配置 + 手动勾选启用
- 插件间通信：服务注册发现 + 事件总线
- 热加载支持：运行中动态加载、卸载、重载插件
- 基于 Qt 的 GUI：插件管理对话框 + 插件自定义页面嵌入宿主窗口

---

## 2. 架构概览

### 2.1 目录结构

```
PluginFramework/
├── PluginSDK/                  # 插件开发 SDK（INTERFACE 目标，头文件）
│   └── include/
│       ├── IPlugin.h
│       ├── IHost.h
│       ├── IPluginService.h
│       ├── IEventHandler.h
│       └── PluginTypes.h
├── Core/                       # 核心框架（静态库）
│   ├── PluginManager.h/.cpp      # 插件加载/卸载/生命周期管理
│   ├── PluginRegistry.h/.cpp     # 服务注册与发现
│   ├── EventBus.h/.cpp           # 事件总线
│   ├── PluginScanner.h/.cpp      # 目录扫描与元数据读取
│   └── ConfigManager.h/.cpp      # 配置文件读写
├── HostApp/                    # 宿主应用（Qt 可执行文件）
│   └── main.cpp / MainWindow / PluginManagerDialog
├── Plugins/                    # 示例插件
│   ├── SamplePlugin/
│   └── CounterPlugin/
└── docs/
```

### 2.2 核心组件交互

```
┌──────────────────────────────────────────────┐
│              Host Application                │
│  ┌─────────────┐  ┌─────────────────────┐    │
│  │ PluginManager│  │   ConfigManager     │    │
│  │ (加载/卸载)  │  │   (plugins.json)    │    │
│  └──────┬──────┘  └─────────────────────┘    │
│         │                                     │
│  ┌──────┴────────────────────────┐            │
│  │       PluginRegistry          │            │
│  │    (服务注册与发现)             │            │
│  │  ┌───────────────────────┐    │            │
│  │  │       EventBus        │    │            │
│  │  │     (事件总线)         │    │            │
│  │  └───────────────────────┘    │            │
│  └────────┬─────────┬────────────┘            │
│           │         │                         │
│    Load/Free       Query/Register              │
│           │         │                         │
│    ┌──────┴─────────┴──────┐                 │
│    │       Plugin DLLs     │                 │
│    │  ┌────┐ ┌────┐ ┌────┐│                 │
│    │  │P1  │ │P2  │ │P3  ││                 │
│    │  └────┘ └────┘ └────┘│                 │
│    └───────────────────────┘                 │
└──────────────────────────────────────────────┘
```

---

## 3. 核心接口设计

### 3.1 插件元数据

```cpp
struct PluginMetadata {
    std::string id;                        // 唯一标识，如 "com.mycompany.sample"
    std::string name;                      // 显示名称
    std::string version;                   // 版本号，如 "1.2.3"
    std::string author;                    // 作者
    std::string description;               // 描述
    std::string apiVersion;                // 兼容的宿主 API 版本
    std::vector<std::string> dependencies; // 依赖的其他插件 ID
};
```

### 3.2 插件基类 IPlugin

所有插件必须继承并实现以下虚函数：

```cpp
class IPlugin {
public:
    virtual ~IPlugin() = default;

    // 元数据
    virtual PluginMetadata GetMetadata() const = 0;

    // 生命周期
    virtual bool Initialize(IHost* host) = 0;  // 获取宿主接口，注册服务
    virtual bool Start() = 0;                   // 开始运行
    virtual bool Stop() = 0;                    // 停止运行
    virtual void Destroy() = 0;                 // 释放资源

    // 可选：对外提供服务接口
    virtual IPluginService* GetService(const std::string& serviceName) {
        return nullptr;
    }

    // 可选：UI 页面
    virtual std::vector<PluginUIPage> GetUIPages() { return {}; }
    virtual QWidget* CreateUIPage(
        const std::string& pageId,
        QWidget* parent = nullptr
    ) { return nullptr; }
};
```

### 3.3 宿主接口 IHost

```cpp
class IHost {
public:
    virtual ~IHost() = default;

    // 配置读写（持久化到 plugins.json）
    virtual std::string GetConfig(const std::string& key,
                                   const std::string& defaultValue = "") = 0;
    virtual void SetConfig(const std::string& key,
                           const std::string& value) = 0;

    // 日志
    virtual void LogInfo(const std::string& msg) = 0;
    virtual void LogWarn(const std::string& msg) = 0;
    virtual void LogError(const std::string& msg) = 0;

    // 服务查找（插件间通信）
    virtual IPluginService* FindService(const std::string& pluginId,
                                         const std::string& serviceName) = 0;

    // 事件总线
    virtual void PublishEvent(const std::string& eventType, void* eventData) = 0;
    virtual void SubscribeEvent(const std::string& eventType,
                                 IEventHandler* handler) = 0;
    virtual void UnsubscribeEvent(const std::string& eventType,
                                   IEventHandler* handler) = 0;
};
```

### 3.4 插件服务接口 IPluginService

```cpp
class IPluginService {
public:
    virtual ~IPluginService() = default;
    virtual std::string GetServiceName() const = 0;
    // 具体服务接口由插件自行扩展，消费者通过 dynamic_cast 使用
};
```

### 3.5 事件处理器 IEventHandler

```cpp
class IEventHandler {
public:
    virtual ~IEventHandler() = default;
    virtual void OnEvent(const std::string& eventType, void* eventData) = 0;
};
```

### 3.6 UI 页面描述

```cpp
struct PluginUIPage {
    std::string pageId;    // 页面唯一标识
    std::string title;     // 导航栏显示标题
    std::string iconPath;  // 图标路径（可选）
};
```

### 3.7 DLL 导出函数约定

每个插件 DLL 必须导出以下 C 函数：

```cpp
extern "C" __declspec(dllexport) IPlugin* CreatePlugin();
extern "C" __declspec(dllexport) void DestroyPlugin(IPlugin* plugin);
```

---

## 4. 插件生命周期与状态机

### 4.1 状态定义

| 状态 | 说明 |
|------|------|
| Unknown | 初始状态，尚未扫描 |
| Discovered | 扫描发现，但未加载 |
| Loaded | DLL 已 LoadLibrary，实例已创建 |
| Ready | Initialize 成功，服务已注册 |
| Running | Start 成功，正在运行 |
| Error | 某阶段失败，记录错误原因 |
| Unloaded | 已卸载 |

### 4.2 状态转换

```
Unknown ──扫描发现──► Discovered ──勾选启用──► Loaded
                                                      │
                                              LoadLibrary
                                              CreatePlugin
                                                      ▼
                                              Ready ◄── Initialize
                                                │
                                          Start │ │ Stop
                                                ▼
                                              Running
                                                │
                                          ┌─────┴─────┐
                                          │ Hot Reload │
                                          └─────┬─────┘
                                                │
                                        Stop → Destroy → Reload → Initialize → Start
```

### 4.3 生命周期方法

- **Load**：`LoadLibrary` → 获取 `CreatePlugin` → 创建实例 → 读取 `GetMetadata()`
- **Initialize**：检查依赖是否满足 → 调用 `IPlugin::Initialize(IHost*)` → 注册服务到 Registry
- **Start**：按依赖拓扑顺序 → 调用 `IPlugin::Start()`
- **Stop**：反向顺序 → 调用 `IPlugin::Stop()`
- **Destroy**：从 Registry 注销服务 → 调用 `IPlugin::Destroy()` → `DestroyPlugin` → `FreeLibrary`

### 4.4 依赖解析

- 加载前检查 `dependencies` 列表，若依赖未加载则递归加载
- 发现循环依赖时拒绝加载，标记状态为 Error

---

## 5. 配置与发现机制

### 5.1 配置文件格式（JSON）

文件路径：`plugins.json`

```json
{
    "pluginFramework": {
        "version": "1.0.0",
        "pluginDir": "plugins",
        "autoLoad": false
    },
    "plugins": {
        "com.example.plugin1": {
            "enabled": true,
            "config": {
                "key1": "value1"
            }
        },
        "com.example.plugin2": {
            "enabled": false,
            "config": {}
        }
    }
}
```

### 5.2 发现流程

1. **读取配置**：加载 `plugins.json`，获取已知插件的启用状态
2. **扫描目录**：遍历 `pluginDir`，对每个 `.dll`：
   - 临时加载（不执行 Initialize）
   - 读取 `GetMetadata()`
   - 卸载
3. **合并结果**：
   - 配置中有 + 扫描到：显示，按配置 `enabled` 决定是否加载
   - 配置中无 + 扫描到：显示，默认 `enabled: false`
   - 配置中有 + 扫描不到：标记为"缺失"
4. **加载启用的插件**：按依赖拓扑顺序逐个 Load → Initialize → Start

---

## 6. 插件间通信

### 6.1 直接服务调用（强类型、同步）

```cpp
// 插件 A 提供服务
class ICounterService : public IPluginService {
public:
    virtual int GetCount() const = 0;
    virtual void Increment() = 0;
};

// 插件 B 调用
auto* service = host->FindService("com.example.counter", "CounterService");
if (auto* counter = dynamic_cast<ICounterService*>(service)) {
    counter->Increment();
}
```

### 6.2 事件总线（松耦合、广播）

```cpp
// 订阅
host->SubscribeEvent("FileDownloaded", this);

// 发布
host->PublishEvent("FileDownloaded", &eventData);
```

事件数据类型约定由发布者和消费者自行协商，使用 `void*` 传递指针。

---

## 7. 热加载设计

### 7.1 触发方式

- **手动触发**：插件管理对话框中勾选/取消勾选后点击"应用更改"
- **文件监控**（可选扩展）：监控 `pluginDir`，DLL 文件变更时自动 Reload

### 7.2 重载流程

```
当前插件 Running
        │
        ▼
    Stop()
        │
        ▼
   Destroy()
        │
        ▼
  FreeLibrary (旧 DLL)
        │
        ▼
  LoadLibrary (新 DLL)
        │
        ▼
  Initialize()
        │
        ▼
   Start()
        │
        ▼
   Running (新)
```

### 7.3 状态保持

热重载不内置自动状态迁移。插件如需保持状态，应在 `Stop()` 中通过 `IHost::SetConfig` 写入配置，在 `Initialize()` 或 `Start()` 中通过 `IHost::GetConfig` 读取恢复。

### 7.4 Windows DLL 文件锁定处理

`FreeLibrary` 后 DLL 文件仍可能被系统锁定，热重载时：
- 先重命名旧 DLL（如 `plugin.dll` → `plugin.dll.old`）
- 加载新 DLL
- 程序退出时清理 `.old` 文件

---

## 8. 错误处理策略

| 场景 | 处理方式 |
|------|----------|
| DLL 加载失败 | 记录错误日志，跳过该插件，不影响其他插件 |
| 依赖缺失 | 拒绝加载，标记 Error，原因"缺少依赖: xxx" |
| 版本不兼容 | 拒绝加载，标记 Error，原因"需要宿主 API v2.0，当前 v1.0" |
| Initialize/Start 失败 | 回退状态（调用 Destroy 若已 Initialize），不影响已加载插件 |
| 循环依赖 | 拒绝加载整组涉及的插件 |
| 热重载失败 | 取消重载，保持旧版本运行 |

---

## 9. GUI 设计（Qt）

### 9.1 宿主主窗口

- **左侧导航栏**：显示已加载且提供 UI 的插件列表（图标 + 标题）
- **内容区**：`QStackedWidget`，切换显示当前选中插件的页面
- **管理入口**：固定菜单项/按钮，打开插件管理对话框

### 9.2 插件管理对话框

- 表格显示所有插件（已启用、已发现、已加载、加载失败、缺失）
- 勾选框控制 `enabled` 状态
- "应用更改"按钮对比当前状态与已加载状态，执行 Load/Unload/Reload
- "刷新"按钮重新扫描目录
- 选中插件时下方显示详细信息（元数据、依赖、错误信息）

### 9.3 插件 UI 集成

- 插件通过 `GetUIPages()` 声明页面，`CreateUIPage()` 返回 `QWidget*`
- QML 插件内部用 `QQuickWidget` 包裹后返回
- 所有跨 DLL 的 Qt 对象在同一 Qt 共享库实例中操作

---

## 10. 构建系统

使用 **CMake**，目标划分：

| Target | 类型 | 说明 |
|--------|------|------|
| `PluginSDK` | INTERFACE | 头文件集合，插件工程依赖 |
| `PluginCore` | STATIC | 核心框架逻辑，HostApp 链接 |
| `HostApp` | EXECUTABLE | Qt 宿主应用示例 |
| `SamplePlugin` | SHARED | 示例插件（日志 + 事件订阅） |
| `CounterPlugin` | SHARED | 示例插件（服务接口 + 计数器） |

依赖：
- `PluginSDK` → Qt6::Widgets（或 Qt5::Widgets）
- `PluginCore` → `PluginSDK`
- `HostApp` → `PluginCore`, Qt6::Widgets
- 各 Plugin → `PluginSDK`, Qt6::Widgets（可选）

---

## 11. 示例插件

### 11.1 SamplePlugin

- 演示生命周期回调 + 事件订阅
- 启动时订阅 `TimerTick` 事件，每次收到打印日志

### 11.2 CounterPlugin

- 演示服务接口暴露 + UI 页面
- 提供 `ICounterService`（GetCount / Increment）
- 提供 UI 页面显示当前计数，带 +1 按钮

---

## 12. 技术栈

- **语言**：C++17
- **构建**：CMake 3.20+
- **GUI**：Qt6（或 Qt5）
- **配置**：nlohmann/json（或 Qt JSON）
- **平台**：Windows（`__declspec(dllexport/dllimport)`）
- **后续扩展**：Linux（`__attribute__((visibility))`）
