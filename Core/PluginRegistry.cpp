#include "PluginRegistry.h"

namespace PluginFramework {

PluginRegistry::PluginRegistry() = default;

PluginRegistry::~PluginRegistry() {
    UnregisterAllServices();
}

void PluginRegistry::RegisterService(const std::string& serviceId, std::shared_ptr<IPluginService> service) {
    if (!service) return;

    auto it = m_services.find(serviceId);
    if (it != m_services.end()) {
        // 服务已存在，更新实现
        it->second.service = service;
    } else {
        ServiceEntry entry;
        entry.service = std::move(service);
        entry.refCount = 0;
        m_services[serviceId] = std::move(entry);
    }
}

void PluginRegistry::UnregisterService(const std::string& serviceId) {
    auto it = m_services.find(serviceId);
    if (it != m_services.end()) {
        m_services.erase(it);
    }
}

std::shared_ptr<IPluginService> PluginRegistry::GetService(const std::string& serviceId) {
    auto it = m_services.find(serviceId);
    if (it != m_services.end()) {
        return it->second.service;
    }
    return nullptr;
}

bool PluginRegistry::HasService(const std::string& serviceId) {
    return m_services.find(serviceId) != m_services.end();
}

std::vector<std::string> PluginRegistry::GetAllServices() {
    std::vector<std::string> result;
    for (const auto& pair : m_services) {
        result.push_back(pair.first);
    }
    return result;
}

void PluginRegistry::AddRef(const std::string& serviceId) {
    auto it = m_services.find(serviceId);
    if (it != m_services.end()) {
        ++it->second.refCount;
    }
}

void PluginRegistry::ReleaseRef(const std::string& serviceId) {
    auto it = m_services.find(serviceId);
    if (it != m_services.end()) {
        --it->second.refCount;
        if (it->second.refCount < 0) {
            it->second.refCount = 0;
        }
    }
}

int PluginRegistry::GetRefCount(const std::string& serviceId) {
    auto it = m_services.find(serviceId);
    if (it != m_services.end()) {
        return it->second.refCount;
    }
    return 0;
}

void PluginRegistry::RegisterServices(const std::map<std::string, std::shared_ptr<IPluginService>>& services) {
    for (const auto& pair : services) {
        RegisterService(pair.first, pair.second);
    }
}

void PluginRegistry::UnregisterAllServices() {
    m_services.clear();
}

} // namespace PluginFramework
