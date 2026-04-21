#pragma once

#include <string>
#include <memory>
#include <vector>

namespace PluginFramework {

// 插件服务接口
// 用于在插件间共享功能
class IPluginService {
public:
    virtual ~IPluginService() = default;

    // 获取服务名称
    virtual std::string GetServiceName() const = 0;
};

// 服务注册表
class IServiceRegistry {
public:
    virtual ~IServiceRegistry() = default;

    // 注册服务
    virtual void RegisterService(const std::string& serviceId, std::shared_ptr<IPluginService> service) = 0;

    // 注销服务
    virtual void UnregisterService(const std::string& serviceId) = 0;

    // 获取服务
    virtual std::shared_ptr<IPluginService> GetService(const std::string& serviceId) = 0;

    // 查询服务是否存在
    virtual bool HasService(const std::string& serviceId) = 0;

    // 获取所有已注册服务
    virtual std::vector<std::string> GetAllServices() = 0;
};

} // namespace PluginFramework
