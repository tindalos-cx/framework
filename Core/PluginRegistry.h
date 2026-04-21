#pragma once

#include "IPluginService.h"
#include <string>
#include <map>
#include <vector>
#include <memory>

namespace PluginFramework {

// 服务注册表
// 负责服务注册与发现
class PluginRegistry : public IServiceRegistry {
public:
    PluginRegistry();
    ~PluginRegistry() override;

    // 服务管理
    void RegisterService(const std::string& serviceId, std::shared_ptr<IPluginService> service) override;
    void UnregisterService(const std::string& serviceId) override;
    std::shared_ptr<IPluginService> GetService(const std::string& serviceId) override;
    bool HasService(const std::string& serviceId) override;
    std::vector<std::string> GetAllServices() override;

    // 服务引用计数
    void AddRef(const std::string& serviceId);
    void ReleaseRef(const std::string& serviceId);
    int GetRefCount(const std::string& serviceId);

    // 批量操作
    void RegisterServices(const std::map<std::string, std::shared_ptr<IPluginService>>& services);
    void UnregisterAllServices();

private:
    struct ServiceEntry {
        std::shared_ptr<IPluginService> service;
        int refCount;
    };

    std::map<std::string, ServiceEntry> m_services;
};

} // namespace PluginFramework
