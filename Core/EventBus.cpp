#include "EventBus.h"
#include <algorithm>

namespace PluginFramework {

EventBus::EventBus() = default;

EventBus::~EventBus() {
    // 清空所有订阅
    std::lock_guard<std::mutex> lock(m_mutex);
    m_subscribers.clear();
}

void EventBus::Subscribe(const std::string& eventType, IEventHandler* handler) {
    if (!handler) return;

    std::lock_guard<std::mutex> lock(m_mutex);

    auto& subscribers = m_subscribers[eventType];

    // 检查是否已存在
    for (auto& sub : subscribers) {
        if (sub.handler == handler) {
            sub.valid = true;
            return;
        }
    }

    subscribers.push_back({handler, true});
}

void EventBus::Unsubscribe(const std::string& eventType, IEventHandler* handler) {
    if (!handler) return;

    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_subscribers.find(eventType);
    if (it == m_subscribers.end()) return;

    auto& subscribers = it->second;
    for (auto& sub : subscribers) {
        if (sub.handler == handler) {
            sub.valid = false;
            break;
        }
    }

    // 清理无效订阅
    subscribers.erase(
        std::remove_if(subscribers.begin(), subscribers.end(),
            [](const Subscription& sub) { return !sub.valid; }),
        subscribers.end()
    );
}

void EventBus::UnsubscribeAll(IEventHandler* handler) {
    if (!handler) return;

    std::lock_guard<std::mutex> lock(m_mutex);

    for (auto& pair : m_subscribers) {
        auto& subscribers = pair.second;
        for (auto& sub : subscribers) {
            if (sub.handler == handler) {
                sub.valid = false;
            }
        }

        // 清理无效订阅
        subscribers.erase(
            std::remove_if(subscribers.begin(), subscribers.end(),
                [](const Subscription& sub) { return !sub.valid; }),
            subscribers.end()
        );
    }
}

size_t EventBus::GetSubscriberCount(const std::string& eventType) {
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_subscribers.find(eventType);
    if (it == m_subscribers.end()) return 0;

    size_t count = 0;
    for (const auto& sub : it->second) {
        if (sub.valid) ++count;
    }
    return count;
}

void EventBus::Publish(const std::string& eventType, void* eventData) {
    std::vector<IEventHandler*> handlers;

    // 复制有效的处理器列表
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        auto it = m_subscribers.find(eventType);
        if (it == m_subscribers.end()) return;

        for (const auto& sub : it->second) {
            if (sub.valid) {
                handlers.push_back(sub.handler);
            }
        }
    }

    // 在锁外调用处理器
    for (auto* handler : handlers) {
        try {
            handler->OnEvent(eventType, eventData);
        } catch (const std::exception& e) {
            // 记录异常日志
        }
    }
}

std::vector<std::string> EventBus::GetAllEventTypes() {
    std::lock_guard<std::mutex> lock(m_mutex);

    std::vector<std::string> result;
    for (const auto& pair : m_subscribers) {
        result.push_back(pair.first);
    }
    return result;
}

} // namespace PluginFramework
