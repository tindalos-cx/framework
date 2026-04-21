#pragma once

#include "IEventHandler.h"
#include <string>
#include <map>
#include <vector>
#include <mutex>

namespace PluginFramework {

// 事件总线实现
// 提供线程安全的事件发布/订阅功能
class EventBus : public IEventBus {
public:
    EventBus();
    ~EventBus() override;

    // 订阅管理
    void Subscribe(const std::string& eventType, IEventHandler* handler) override;
    void Unsubscribe(const std::string& eventType, IEventHandler* handler) override;
    void UnsubscribeAll(IEventHandler* handler) override;
    size_t GetSubscriberCount(const std::string& eventType) override;

    // 事件发布
    void Publish(const std::string& eventType, void* eventData) override;

    // 查询
    std::vector<std::string> GetAllEventTypes();

private:
    struct Subscription {
        IEventHandler* handler;
        bool valid;
    };

    std::mutex m_mutex;
    std::map<std::string, std::vector<Subscription>> m_subscribers;
};

} // namespace PluginFramework
