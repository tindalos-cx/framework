#include "HostAppAdapter.h"
#include "IPluginService.h"
#include <iostream>
#include <ctime>

namespace PluginFramework {

HostAppAdapter::HostAppAdapter() = default;

HostAppAdapter::~HostAppAdapter() {
    Shutdown();
}

bool HostAppAdapter::Initialize() {
    return true;
}

void HostAppAdapter::Shutdown() {
    m_eventBus.UnsubscribeAll(nullptr);
    m_registry.UnregisterAllServices();
}

std::string HostAppAdapter::GetConfig(const std::string& key, const std::string& defaultValue) {
    // TODO: 从 ConfigManager 获取配置
    return defaultValue;
}

void HostAppAdapter::SetConfig(const std::string& key, const std::string& value) {
    // TODO: 保存到 ConfigManager
}

void HostAppAdapter::LogInfo(const std::string& msg) {
    std::cout << "[INFO] " << msg << std::endl;
}

void HostAppAdapter::LogWarn(const std::string& msg) {
    std::cout << "[WARN] " << msg << std::endl;
}

void HostAppAdapter::LogError(const std::string& msg) {
    std::cerr << "[ERROR] " << msg << std::endl;
}

IPluginService* HostAppAdapter::FindService(const std::string& pluginId, const std::string& serviceName) {
    std::string fullServiceId = pluginId + "." + serviceName;
    auto service = m_registry.GetService(fullServiceId);
    return service.get();
}

void HostAppAdapter::PublishEvent(const std::string& eventType, void* eventData) {
    m_eventBus.Publish(eventType, eventData);
}

void HostAppAdapter::SubscribeEvent(const std::string& eventType, IEventHandler* handler) {
    m_eventBus.Subscribe(eventType, handler);
}

void HostAppAdapter::UnsubscribeEvent(const std::string& eventType, IEventHandler* handler) {
    m_eventBus.Unsubscribe(eventType, handler);
}

} // namespace PluginFramework
