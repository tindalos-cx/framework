#pragma once

#include "PluginTypes.h"
#include <string>
#include <memory>
#include <vector>
#include <map>
#include <functional>

namespace PluginFramework {

// 前向声明
class IPlugin;
class IHost;
class PluginScanner;
class ConfigManager;

// 插件加载信息
struct PluginInfo {
    std::string pluginId;
    std::string filePath;
    PluginMetadata metadata;
    PluginState state;
    std::shared_ptr<IPlugin> instance;
    void* moduleHandle;  // 平台相关的模块句柄
    std::string errorMessage;
};

// 插件生命周期回调
struct PluginLifecycleCallbacks {
    std::function<void(const std::string& pluginId)> onLoading;
    std::function<void(const std::string& pluginId)> onLoaded;
    std::function<void(const std::string& pluginId)> onStarted;
    std::function<void(const std::string& pluginId)> onStopping;
    std::function<void(const std::string& pluginId)> onStopped;
    std::function<void(const std::string& pluginId, PluginError error)> onError;
};

// 插件管理器
// 负责插件的加载、卸载、生命周期管理
class PluginManager {
public:
    explicit PluginManager(IHost* host);
    ~PluginManager();

    // 生命周期
    bool Initialize();
    void Shutdown();

    // 插件加载/卸载
    PluginError LoadPlugin(const std::string& filePath);
    PluginError UnloadPlugin(const std::string& pluginId);
    PluginError ReloadPlugin(const std::string& pluginId);

    // 插件查询
    std::shared_ptr<IPlugin> GetPlugin(const std::string& pluginId);
    std::vector<std::shared_ptr<IPlugin>> GetAllPlugins();
    bool IsPluginLoaded(const std::string& pluginId);
    const PluginInfo* GetPluginInfo(const std::string& pluginId);

    // 插件扫描
    std::vector<PluginMetadata> ScanPlugins(const std::string& directory);
    void LoadEnabledPlugins();

    // 插件配置
    void LoadEnabledList();
    void SaveEnabledList();

    // 事件回调
    void SetLifecycleCallbacks(const PluginLifecycleCallbacks& callbacks);

    // 错误处理
    std::string GetLastError() const;
    void ClearError();

private:
    // 内部方法
    PluginError LoadPluginInternal(const std::string& filePath);
    bool InitializePlugin(std::shared_ptr<IPlugin> plugin);
    bool StartPlugin(std::shared_ptr<IPlugin> plugin);
    void StopPlugin(std::shared_ptr<IPlugin> plugin);
    void UnloadPluginInternal(const std::string& pluginId);

    bool ResolveDependencies(const PluginMetadata& metadata);
    std::vector<std::string> GetLoadOrder();

    void SetPluginState(const std::string& pluginId, PluginState state);
    void NotifyLifecycleEvent(const std::string& pluginId, PluginEventType eventType, PluginError error = PluginError::Success);

    // 平台相关实现
#ifdef _WIN32
    void* LoadModule(const std::string& filePath);
    void UnloadModule(void* module);
#else
    void* LoadModule(const std::string& filePath);
    void UnloadModule(void* module);
#endif

    // 成员变量
    class Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace PluginFramework
