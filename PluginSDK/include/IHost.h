#pragma once

#include <string>
#include <memory>

namespace PluginFramework {

// 前向声明
class IPluginService;
class IEventHandler;

// 宿主应用接口
// 插件通过此接口与宿主交互
class IHost {
public:
    virtual ~IHost() = default;

    // ==================== 配置管理 ====================
    // 读取配置值
    virtual std::string GetConfig(const std::string& key, const std::string& defaultValue = "") = 0;
    // 设置配置值
    virtual void SetConfig(const std::string& key, const std::string& value) = 0;

    // ==================== 日志 ====================
    virtual void LogInfo(const std::string& msg) = 0;
    virtual void LogWarn(const std::string& msg) = 0;
    virtual void LogError(const std::string& msg) = 0;

    // ==================== 服务管理 ====================
    // 查找服务
    virtual IPluginService* FindService(const std::string& pluginId, const std::string& serviceName) = 0;

    // ==================== 事件系统 ====================
    // 发布事件
    virtual void PublishEvent(const std::string& eventType, void* eventData) = 0;
    // 订阅事件
    virtual void SubscribeEvent(const std::string& eventType, IEventHandler* handler) = 0;
    // 取消订阅事件
    virtual void UnsubscribeEvent(const std::string& eventType, IEventHandler* handler) = 0;
};

} // namespace PluginFramework
