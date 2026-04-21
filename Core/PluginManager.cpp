#include "PluginManager.h"
#include "IPlugin.h"
#include "IHost.h"
#include "PluginScanner.h"
#include "ConfigManager.h"
#include <algorithm>
#include <filesystem>
#include <sstream>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <dlfcn.h>
#endif

namespace PluginFramework {

class PluginManager::Impl {
public:
    IHost* host;
    std::map<std::string, PluginInfo> plugins;
    std::vector<std::string> enabledPlugins;
    std::unique_ptr<PluginScanner> scanner;
    std::unique_ptr<ConfigManager> configManager;
    PluginLifecycleCallbacks callbacks;
    std::string lastError;
    bool initialized = false;
};

PluginManager::PluginManager(IHost* host)
    : m_impl(std::make_unique<Impl>()) {
    m_impl->host = host;
    m_impl->scanner = std::make_unique<PluginScanner>();
    m_impl->configManager = std::make_unique<ConfigManager>();
}

PluginManager::~PluginManager() {
    Shutdown();
}

bool PluginManager::Initialize() {
    if (m_impl->initialized) {
        return true;
    }

    // 初始化配置管理器
    if (!m_impl->configManager->Initialize()) {
        m_impl->lastError = "Failed to initialize config manager";
        return false;
    }

    // 加载启用的插件列表
    LoadEnabledList();

    m_impl->initialized = true;
    return true;
}

void PluginManager::Shutdown() {
    if (!m_impl->initialized) {
        return;
    }

    // 按反向顺序停止并卸载所有插件
    for (auto it = m_impl->plugins.rbegin(); it != m_impl->plugins.rend(); ++it) {
        const auto& pluginId = it->first;
        auto& info = it->second;

        if (info.instance) {
            if (info.state == PluginState::Running) {
                StopPlugin(info.instance);
            }
            if (info.state == PluginState::Loaded || info.state == PluginState::Ready) {
                UnloadPluginInternal(pluginId);
            }
        }
    }

    m_impl->plugins.clear();
    m_impl->initialized = false;
}

PluginError PluginManager::LoadPlugin(const std::string& filePath) {
    return LoadPluginInternal(filePath);
}

PluginError PluginManager::UnloadPlugin(const std::string& pluginId) {
    auto it = m_impl->plugins.find(pluginId);
    if (it == m_impl->plugins.end()) {
        m_impl->lastError = "Plugin not found: " + pluginId;
        return PluginError::FileNotFound;
    }

    auto& info = it->second;

    if (info.state == PluginState::Running) {
        StopPlugin(info.instance);
    }

    UnloadPluginInternal(pluginId);
    return PluginError::Success;
}

PluginError PluginManager::ReloadPlugin(const std::string& pluginId) {
    auto it = m_impl->plugins.find(pluginId);
    if (it == m_impl->plugins.end()) {
        m_impl->lastError = "Plugin not found: " + pluginId;
        return PluginError::FileNotFound;
    }

    auto filePath = it->second.filePath;

    UnloadPlugin(pluginId);
    return LoadPlugin(filePath);
}

std::shared_ptr<IPlugin> PluginManager::GetPlugin(const std::string& pluginId) {
    auto it = m_impl->plugins.find(pluginId);
    if (it != m_impl->plugins.end()) {
        return it->second.instance;
    }
    return nullptr;
}

std::vector<std::shared_ptr<IPlugin>> PluginManager::GetAllPlugins() {
    std::vector<std::shared_ptr<IPlugin>> result;
    for (auto& pair : m_impl->plugins) {
        if (pair.second.instance) {
            result.push_back(pair.second.instance);
        }
    }
    return result;
}

bool PluginManager::IsPluginLoaded(const std::string& pluginId) {
    auto it = m_impl->plugins.find(pluginId);
    return it != m_impl->plugins.end() && it->second.instance != nullptr;
}

const PluginInfo* PluginManager::GetPluginInfo(const std::string& pluginId) {
    auto it = m_impl->plugins.find(pluginId);
    if (it != m_impl->plugins.end()) {
        return &it->second;
    }
    return nullptr;
}

std::vector<PluginMetadata> PluginManager::ScanPlugins(const std::string& directory) {
    return m_impl->scanner->Scan(directory);
}

void PluginManager::LoadEnabledPlugins() {
    for (const auto& pluginId : m_impl->enabledPlugins) {
        // 查找插件文件路径
        // TODO: 实现插件路径查找逻辑
    }
}

void PluginManager::LoadEnabledList() {
    m_impl->enabledPlugins = m_impl->configManager->GetEnabledPlugins();
}

void PluginManager::SaveEnabledList() {
    m_impl->configManager->SetEnabledPlugins(m_impl->enabledPlugins);
    m_impl->configManager->Save();
}

void PluginManager::SetLifecycleCallbacks(const PluginLifecycleCallbacks& callbacks) {
    m_impl->callbacks = callbacks;
}

std::string PluginManager::GetLastError() const {
    return m_impl->lastError;
}

void PluginManager::ClearError() {
    m_impl->lastError.clear();
}

PluginError PluginManager::LoadPluginInternal(const std::string& filePath) {
    // 加载动态库
    void* module = LoadModule(filePath);
    if (!module) {
        m_impl->lastError = "Failed to load module: " + filePath;
        return PluginError::FileNotFound;
    }

    // 获取插件工厂函数
#ifdef _WIN32
    auto factory = reinterpret_cast<PluginFactoryFunc>(
        GetProcAddress(static_cast<HMODULE>(module), "GetPluginFactory"));
#else
    auto factory = reinterpret_cast<PluginFactoryFunc>(
        dlsym(module, "GetPluginFactory"));
#endif

    if (!factory) {
        m_impl->lastError = "Failed to get plugin factory: " + filePath;
        UnloadModule(module);
        return PluginError::SymbolNotFound;
    }

    // 获取元数据
    PluginMetadata metadata;
#ifdef _WIN32
    auto metadataFunc = reinterpret_cast<decltype(&GetPluginMetadata)>(
        GetProcAddress(static_cast<HMODULE>(module), "GetPluginMetadata"));
#else
    auto metadataFunc = reinterpret_cast<decltype(&GetPluginMetadata)>(
        dlsym(module, "GetPluginMetadata"));
#endif

    if (metadataFunc && metadataFunc()) {
        metadata = *metadataFunc();
    } else {
        m_impl->lastError = "Failed to get plugin metadata: " + filePath;
        UnloadModule(module);
        return PluginError::InvalidFormat;
    }

    // 检查依赖
    if (!ResolveDependencies(metadata)) {
        UnloadModule(module);
        return PluginError::DependencyMissing;
    }

    // 创建插件实例
    auto plugin = std::shared_ptr<IPlugin>(factory());
    if (!plugin) {
        m_impl->lastError = "Failed to create plugin instance: " + filePath;
        UnloadModule(module);
        return PluginError::InitFailed;
    }

    // 存储插件信息
    PluginInfo info;
    info.pluginId = metadata.id;
    info.filePath = filePath;
    info.metadata = metadata;
    info.state = PluginState::Loaded;
    info.instance = plugin;
    info.moduleHandle = module;

    m_impl->plugins[metadata.id] = info;
    NotifyLifecycleEvent(metadata.id, PluginEventType::Loading);

    // 初始化插件
    if (!InitializePlugin(plugin)) {
        plugin->Stop();
        UnloadPluginInternal(metadata.id);
        return PluginError::InitFailed;
    }

    SetPluginState(metadata.id, PluginState::Ready);
    NotifyLifecycleEvent(metadata.id, PluginEventType::Loaded);

    // 启动插件
    if (!StartPlugin(plugin)) {
        return PluginError::InitFailed;
    }

    return PluginError::Success;
}

bool PluginManager::InitializePlugin(std::shared_ptr<IPlugin> plugin) {
    SetPluginState(plugin->GetMetadata().id, PluginState::Ready);
    NotifyLifecycleEvent(plugin->GetMetadata().id, PluginEventType::Initializing);

    if (!plugin->Initialize(m_impl->host)) {
        m_impl->lastError = "Plugin initialization failed: " + plugin->GetMetadata().id;
        NotifyLifecycleEvent(plugin->GetMetadata().id, PluginEventType::Error, PluginError::InitFailed);
        return false;
    }

    return true;
}

bool PluginManager::StartPlugin(std::shared_ptr<IPlugin> plugin) {
    try {
        bool success = plugin->Start();
        if (success) {
            SetPluginState(plugin->GetMetadata().id, PluginState::Running);
            NotifyLifecycleEvent(plugin->GetMetadata().id, PluginEventType::Started);
        }
        return success;
    } catch (const std::exception& e) {
        m_impl->lastError = "Plugin start failed: " + std::string(e.what());
        NotifyLifecycleEvent(plugin->GetMetadata().id, PluginEventType::Error, PluginError::InitFailed);
        return false;
    }
}

void PluginManager::StopPlugin(std::shared_ptr<IPlugin> plugin) {
    if (!plugin) return;

    const auto& pluginId = plugin->GetMetadata().id;
    SetPluginState(pluginId, PluginState::Ready);
    NotifyLifecycleEvent(pluginId, PluginEventType::Stopping);

    try {
        plugin->Stop();
        SetPluginState(pluginId, PluginState::Loaded);
        NotifyLifecycleEvent(pluginId, PluginEventType::Stopped);
    } catch (const std::exception& e) {
        m_impl->lastError = "Plugin stop failed: " + std::string(e.what());
    }
}

void PluginManager::UnloadPluginInternal(const std::string& pluginId) {
    auto it = m_impl->plugins.find(pluginId);
    if (it == m_impl->plugins.end()) return;

    auto& info = it->second;
    SetPluginState(pluginId, PluginState::Unloaded);
    NotifyLifecycleEvent(pluginId, PluginEventType::Unloading);

    info.instance->Destroy();
    info.instance.reset();

    if (info.moduleHandle) {
        UnloadModule(info.moduleHandle);
    }

    m_impl->plugins.erase(it);
    NotifyLifecycleEvent(pluginId, PluginEventType::Unloaded);
}

bool PluginManager::ResolveDependencies(const PluginMetadata& metadata) {
    for (const auto& depId : metadata.dependencies) {
        if (!IsPluginLoaded(depId)) {
            m_impl->lastError = "Missing dependency: " + depId + " for plugin: " + metadata.id;
            return false;
        }
    }
    return true;
}

std::vector<std::string> PluginManager::GetLoadOrder() {
    // TODO: 实现拓扑排序获取加载顺序
    std::vector<std::string> order;
    for (const auto& pair : m_impl->plugins) {
        order.push_back(pair.first);
    }
    return order;
}

void PluginManager::SetPluginState(const std::string& pluginId, PluginState state) {
    auto it = m_impl->plugins.find(pluginId);
    if (it != m_impl->plugins.end()) {
        it->second.state = state;
    }
}

void PluginManager::NotifyLifecycleEvent(const std::string& pluginId, PluginEventType eventType, PluginError error) {
    if (eventType == PluginEventType::Loading && m_impl->callbacks.onLoading) {
        m_impl->callbacks.onLoading(pluginId);
    } else if (eventType == PluginEventType::Loaded && m_impl->callbacks.onLoaded) {
        m_impl->callbacks.onLoaded(pluginId);
    } else if (eventType == PluginEventType::Started && m_impl->callbacks.onStarted) {
        m_impl->callbacks.onStarted(pluginId);
    } else if (eventType == PluginEventType::Error && m_impl->callbacks.onError) {
        m_impl->callbacks.onError(pluginId, error);
    }
}

#ifdef _WIN32
void* PluginManager::LoadModule(const std::string& filePath) {
    return LoadLibraryA(filePath.c_str());
}

void PluginManager::UnloadModule(void* module) {
    if (module) {
        FreeLibrary(static_cast<HMODULE>(module));
    }
}
#else
void* PluginManager::LoadModule(const std::string& filePath) {
    return dlopen(filePath.c_str(), RTLD_LAZY);
}

void PluginManager::UnloadModule(void* module) {
    if (module) {
        dlclose(module);
    }
}
#endif

} // namespace PluginFramework
