# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## 项目概述

这是一个基于进程内 DLL 的 C++ 桌面应用插件框架，使用 Qt6 提供 GUI 支持，CMake 构建。宿主应用通过加载 DLL 动态扩展功能，插件遵循统一的 `IPlugin` 接口。

## 构建命令

依赖：CMake >= 3.16、**Qt 6.0+**、C++17 编译器（MSVC 2022 推荐）。

由于 Qt6 通常不会自动被 CMake 找到，**配置时必须通过 `CMAKE_PREFIX_PATH` 指定 Qt6 安装路径**（以本机为例，`E:/app/qt/6.8.3/msvc2022_64`）：

```bash
# 配置（必须指定 Qt6 路径）
cmake -B build -S . -DCMAKE_PREFIX_PATH="E:/app/qt/6.8.3/msvc2022_64"

# 编译
cmake --build build --config Release

# 运行宿主应用
# Windows (Release 配置):
./build/bin/Release/PluginHostApp.exe
# Linux/macOS:
./build/bin/PluginHostApp
```

构建后插件 DLL 会自动复制到 `build/bin/plugins/Release/` 目录。

### 运行时部署

编译完成后，可执行文件依赖 Qt6 运行时 DLL，**必须先部署才能运行**：

```bash
# 部署 Qt6 依赖到输出目录
E:/app/qt/6.8.3/msvc2022_64/bin/windeployqt.exe build/bin/Release/PluginHostApp.exe --dir build/bin/Release/
```

**MSVC 运行时**：目标机器如果没有安装 Visual Studio，还需要 `MSVCP140.dll`、`VCRUNTIME140.dll` 等，可让对方安装 [VC++ Redistributable](https://aka.ms/vs/17/release/vc_redist.x64.exe)。

## 项目架构

### 三层模块结构

```
PluginSDK  (INTERFACE 库，纯头文件)
    ↑ 被插件和 Core 依赖
Core       (STATIC 库，框架实现)
    ↑ 被 HostApp 依赖
HostApp    (EXECUTABLE，Qt GUI 应用)
```

- **PluginSDK/**：定义插件接口契约。关键头文件：`IPlugin.h`（插件基类）、`IHost.h`（宿主接口，插件通过它与宿主交互）、`IEventHandler.h` / `IPluginService.h`（通信抽象）、`PluginTypes.h`（元数据、状态、错误码枚举）。
- **Core/**：框架核心实现，编译为静态库 `PluginCore`。
  - `PluginManager`：插件生命周期管理（加载、初始化、启动、停止、卸载、重载）。使用 PIMPL 模式（`Impl` 类）。
  - `PluginScanner`：扫描目录中的插件 DLL 并读取 `plugin.json` 元数据。
  - `PluginRegistry`：服务注册表，支持服务注册、发现、引用计数。
  - `EventBus`：线程安全的事件发布/订阅总线。
  - `HostAppAdapter`：`IHost` 的实现，桥接插件与宿主子系统。
  - `ConfigManager`：读写插件启用列表和通用配置。
- **HostApp/**：Qt 宿主应用，包含 `MainWindow` 和 `PluginManagerDialog`。
- **Plugins/**：示例插件目录，每个插件是一个独立的 `SHARED` 库。

### 插件生命周期

状态流转：`Discovered` → `Loaded` → `Ready` → `Running` → `Stopping` → `Stopped` → `Unloading` → `Unloaded`

`PluginManager` 在加载时自动解析依赖并按拓扑顺序启动。`HostApp/main.cpp` 展示了标准初始化顺序：创建 `HostAppAdapter` → 创建 `PluginManager` → 加载并启动插件。

### 插件间通信

1. **服务注册发现**：插件通过 `IPlugin::GetService()` 暴露服务，其他插件通过 `IHost::FindService()` 查找。底层由 `PluginRegistry` 管理，支持引用计数防止过早卸载。
2. **事件总线**：通过 `IHost::PublishEvent()` / `SubscribeEvent()` 进行松耦合通信。`EventBus` 内部使用 `std::mutex` 保证线程安全。

### 插件开发约定

每个插件必须：
1. 继承 `PluginFramework::IPlugin` 并实现纯虚方法。
2. 在 `extern "C"` 块中导出三个 C 函数：`GetPluginFactory()`、`GetPluginAPIVersion()`、`GetPluginMetadata()`。
3. 提供 `plugin.json` 描述元数据（id、version、dependencies 等）。
4. 在 `CMakeLists.txt` 中定义 `PLUGIN_EXPORTS` 以正确导出符号（触发 `PLUGIN_DECL` → `__declspec(dllexport)`）。
5. 仅链接 `PluginFramework::PluginSDK`（头文件库），不链接 `PluginCore`。

参考现有示例：`Plugins/SamplePlugin/` 和 `Plugins/CounterPlugin/`。

### 依赖与工具链

- **Qt6**：`Core` 和 `Widgets` 模块必需。启用 `AUTOMOC` / `AUTORCC` / `AUTOUIC`。配置时需通过 `CMAKE_PREFIX_PATH` 指定 Qt6 安装路径。
- **nlohmann/json**：通过 `FetchContent` 在配置时自动下载（v3.11.3），用于解析 `plugin.json`。
- 所有输出统一放到 `build/bin/` 和 `build/lib/`。

## 注意事项

- 目前项目**没有测试框架**和测试用例。
- 热重载（Reload）在 `PluginManager` 中有接口定义，但实现细节在 PIMPL 中。
- `PluginManagerDialog` 提供 GUI 方式管理插件的启用/禁用/加载/卸载。
- 框架目前仅实现到基础功能阶段，UI 页面嵌入（`CreateUIPage`）接口已定义但示例插件返回空实现。
