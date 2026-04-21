#pragma once

#include <string>
#include <any>

namespace PluginFramework {

// 事件处理器接口
// 用于处理插件间的事件通信
class IEventHandler {
public:
    virtual ~IEventHandler() = default;

    // 处理事件
    virtual void OnEvent(const std::string& eventType, void* eventData) = 0;

    // 获取处理器名称（用于调试）
    virtual const std::string& GetHandlerName() const = 0;
};

// 事件总线
class IEventBus {
public:
    virtual ~IEventBus() = default;

    // 订阅事件
    virtual void Subscribe(const std::string& eventType, IEventHandler* handler) = 0;

    // 取消订阅
    virtual void Unsubscribe(const std::string& eventType, IEventHandler* handler) = 0;

    // 发布事件
    virtual void Publish(const std::string& eventType, void* eventData) = 0;

    // 取消订阅所有事件
    virtual void UnsubscribeAll(IEventHandler* handler) = 0;

    // 获取订阅者数量
    virtual size_t GetSubscriberCount(const std::string& eventType) = 0;
};

// 常用事件类型常量
namespace EventTypes {
    // 系统事件
    constexpr const char* PluginLoaded = "plugin.loaded";
    constexpr const char* PluginUnloaded = "plugin.unloaded";
    constexpr const char* PluginError = "plugin.error";

    // 应用事件
    constexpr const char* AppStarting = "app.starting";
    constexpr const char* AppStarted = "app.started";
    constexpr const char* AppClosing = "app.closing";
    constexpr const char* AppClosed = "app.closed";

    // UI 事件
    constexpr const char* ThemeChanged = "ui.theme_changed";
    constexpr const char* LanguageChanged = "ui.language_changed";
    constexpr const char* WindowStateChanged = "ui.window_state_changed";
}

} // namespace PluginFramework
