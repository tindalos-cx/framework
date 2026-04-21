#pragma once

#include "IHost.h"
#include "PluginRegistry.h"
#include "EventBus.h"
#include <memory>
#include <string>

namespace PluginFramework {

// 宿主应用适配器
// 实现 IHost 接口，为插件提供宿主功能
class HostAppAdapter : public IHost {
public:
    HostAppAdapter();
    ~HostAppAdapter() override;

    // 初始化
    bool Initialize();
    void Shutdown();

    // 配置管理
    std::string GetConfig(const std::string& key, const std::string& defaultValue = "") override;
    void SetConfig(const std::string& key, const std::string& value) override;

    // 日志
    void LogInfo(const std::string& msg) override;
    void LogWarn(const std::string& msg) override;
    void LogError(const std::string& msg) override;

    // 服务管理
    IPluginService* FindService(const std::string& pluginId, const std::string& serviceName) override;

    // 事件系统
    void PublishEvent(const std::string& eventType, void* eventData) override;
    void SubscribeEvent(const std::string& eventType, IEventHandler* handler) override;
    void UnsubscribeEvent(const std::string& eventType, IEventHandler* handler) override;

    // 子系统访问
    PluginRegistry* GetRegistry() { return &m_registry; }
    EventBus* GetEventBus() { return &m_eventBus; }

private:
    PluginRegistry m_registry;
    EventBus m_eventBus;
    std::string m_lastError;
};

} // namespace PluginFramework
