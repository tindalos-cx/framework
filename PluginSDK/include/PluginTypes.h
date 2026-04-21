#pragma once

#include <string>
#include <vector>
#include <memory>
#include <functional>

namespace PluginFramework {

// 插件元数据
struct PluginMetadata {
    std::string id;                              // 唯一标识，如 "com.mycompany.sample"
    std::string name;                            // 显示名称
    std::string version;                          // 版本号，如 "1.2.3"
    std::string author;                           // 作者
    std::string description;                      // 描述
    std::string apiVersion;                       // 兼容的宿主 API 版本
    std::vector<std::string> dependencies;        // 依赖的其他插件 ID
    std::string entrySymbol;                      // 入口函数符号，默认 "GetPluginFactory"
};

// UI 页面配置
struct PluginUIPage {
    std::string pageId;                          // 页面 ID
    std::string title;                           // 页面标题
    std::string iconPath;                        // 图标路径
};

// 插件加载状态
enum class PluginState {
    Unknown,        // 未知
    Discovered,     // 已发现
    Loaded,         // 已加载
    Ready,          // 就绪
    Running,        // 运行中
    Error,          // 错误
    Unloaded        // 已卸载
};

// 插件加载错误码
enum class PluginError {
    Success = 0,
    FileNotFound,
    InvalidFormat,
    IncompatibleAPI,
    DependencyMissing,
    DependencyVersionMismatch,
    InitFailed,
    SymbolNotFound,
    AlreadyLoaded,
    StillReferenced,
    UnknownError
};

// 插件事件类型
enum class PluginEventType {
    Loading,       // 插件开始加载
    Loaded,        // 插件加载完成
    Initializing,  // 插件开始初始化
    Started,       // 插件启动完成
    Stopping,      // 插件开始停止
    Stopped,       // 插件停止完成
    Unloading,     // 插件开始卸载
    Unloaded,      // 插件卸载完成
    Error          // 插件发生错误
};

// 插件事件数据
struct PluginEventData {
    std::string pluginId;
    PluginEventType eventType;
    PluginError error;
    std::string message;
    void* userData;
};

// 插件事件回调
using PluginEventCallback = std::function<void(const PluginEventData&)>;

} // namespace PluginFramework
