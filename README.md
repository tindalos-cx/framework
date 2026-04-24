# C++ DLL Plugin Framework

基于进程内 DLL 的通用桌面应用插件框架。

## 功能特性

- 宿主应用通过加载 DLL 动态扩展功能
- 插件遵循统一的 `IPlugin` 基类接口
- 配置驱动的发现机制：扫描目录 + JSON 配置 + 手动勾选启用
- 插件间通信：服务注册发现 + 事件总线
- 热加载支持：运行中动态加载、卸载、重载插件
- 基于 Qt 的 GUI：插件管理对话框 + 插件自定义页面嵌入宿主窗口

## 项目结构

```
PluginFramework/
├── PluginSDK/           # 插件开发 SDK（头文件）
├── Core/                # 核心框架（静态库）
├── HostApp/             # 宿主应用（Qt 可执行文件）
├── Plugins/             # 示例插件
├── CMakeLists.txt       # 顶层 CMake 配置
└── README.md
```

## 快速开始

### 环境要求

- CMake >= 3.16
- **Qt 6.0+**
- C++17 编译器（GCC 9+, MSVC 2022+, Clang 12+）
- Windows: Visual Studio 2022+

### 构建步骤

Qt6 需要手动指定安装路径（`CMAKE_PREFIX_PATH`）：

```bash
# 创建构建目录
mkdir build && cd build

# 配置项目（Windows 示例路径，请替换为你本地的 Qt6 路径）
cmake .. -DCMAKE_PREFIX_PATH="E:/app/qt/6.8.3/msvc2022_64"

# 编译
cmake --build . --config Release

# 运行宿主应用
./bin/Release/PluginHostApp
```

### 运行时部署

编译出的可执行文件依赖 Qt6 运行时 DLL，需要先用 `windeployqt` 部署后才能运行：

```bash
# 部署 Qt6 依赖到输出目录（路径替换为你本地的 Qt6）
E:/app/qt/6.8.3/msvc2022_64/bin/windeployqt.exe build/bin/Release/PluginHostApp.exe --dir build/bin/Release/
```

**MSVC 运行时**：目标机器如果没有安装 Visual Studio，还需要带上 `MSVCP140.dll`、`VCRUNTIME140.dll` 等运行时库，或让对方安装 [VC++ Redistributable](https://aka.ms/vs/17/release/vc_redist.x64.exe)。

### 开发新插件

1. 在 `Plugins/` 目录下创建新插件目录
2. 继承 `IPlugin` 基类实现插件
3. 在插件目录创建 `CMakeLists.txt`
4. 在顶层 `CMakeLists.txt` 中添加插件

## 许可证

MIT License
