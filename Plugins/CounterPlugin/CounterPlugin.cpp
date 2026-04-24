#include "CounterPlugin.h"
#include <iostream>

CounterPlugin::CounterPlugin()
    : m_host(nullptr)
    , m_counter(0) {
    m_metadata.id = "com.pluginframework.counter";
    m_metadata.name = "Counter Plugin";
    m_metadata.version = "1.0.0";
    m_metadata.author = "PluginFramework";
    m_metadata.description = "A simple counter plugin demonstrating plugin services";
    m_metadata.apiVersion = "1.0.0";
}

CounterPlugin::~CounterPlugin() = default;

PluginFramework::PluginMetadata CounterPlugin::GetMetadata() const {
    return m_metadata;
}

bool CounterPlugin::Initialize(PluginFramework::IHost* host) {
    m_host = host;
    if (m_host) {
        m_host->LogInfo("CounterPlugin: Initializing...");
    }
    return true;
}

bool CounterPlugin::Start() {
    if (m_host) {
        m_host->LogInfo("CounterPlugin: Started with count = " + std::to_string(m_counter));
    }
    return true;
}

bool CounterPlugin::Stop() {
    if (m_host) {
        m_host->LogInfo("CounterPlugin: Stopped with count = " + std::to_string(m_counter));
    }
    return true;
}

void CounterPlugin::Destroy() {
    // 释放资源
}

PluginFramework::IPluginService* CounterPlugin::GetService(const std::string& serviceName) {
    // 示例：返回 nullptr，插件暂不提供服务
    return nullptr;
}

std::vector<PluginFramework::PluginUIPage> CounterPlugin::GetUIPages() {
    return {};
}

QWidget* CounterPlugin::CreateUIPage(const std::string& pageId, QWidget* parent) {
    return nullptr;
}

void CounterPlugin::Increment() {
    ++m_counter;
    if (m_host) {
        m_host->LogInfo("CounterPlugin: Count incremented to " + std::to_string(m_counter));
    }
}

void CounterPlugin::Decrement() {
    --m_counter;
    if (m_host) {
        m_host->LogInfo("CounterPlugin: Count decremented to " + std::to_string(m_counter));
    }
}

int CounterPlugin::GetCount() const {
    return m_counter;
}

void CounterPlugin::Reset() {
    m_counter = 0;
    if (m_host) {
        m_host->LogInfo("CounterPlugin: Count reset to 0");
    }
}

// 插件工厂函数
extern "C" {

PLUGIN_DECL PluginFramework::IPlugin* GetPluginFactory() {
    return new CounterPlugin();
}

PLUGIN_DECL const char* GetPluginAPIVersion() {
    return "1.0.0";
}

PLUGIN_DECL const PluginFramework::PluginMetadata* GetPluginMetadata() {
    static PluginFramework::PluginMetadata metadata = {
        "com.pluginframework.counter",
        "Counter Plugin",
        "1.0.0",
        "PluginFramework",
        "A simple counter plugin demonstrating plugin services",
        "1.0.0",
        {},
        "GetPluginFactory"
    };
    return &metadata;
}

} // extern "C"
