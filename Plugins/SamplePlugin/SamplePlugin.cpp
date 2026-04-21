#include "SamplePlugin.h"

SamplePlugin::SamplePlugin()
    : m_host(nullptr) {
    m_metadata.id = "com.pluginframework.sample";
    m_metadata.name = "Sample Plugin";
    m_metadata.version = "1.0.0";
    m_metadata.author = "PluginFramework";
    m_metadata.description = "A sample plugin demonstrating the plugin framework";
    m_metadata.apiVersion = "1.0.0";
}

SamplePlugin::~SamplePlugin() = default;

PluginFramework::PluginMetadata SamplePlugin::GetMetadata() const {
    return m_metadata;
}

bool SamplePlugin::Initialize(PluginFramework::IHost* host) {
    m_host = host;
    if (m_host) {
        m_host->LogInfo("SamplePlugin: Initializing...");
    }
    return true;
}

bool SamplePlugin::Start() {
    if (m_host) {
        m_host->LogInfo("SamplePlugin: Started");
    }
    return true;
}

bool SamplePlugin::Stop() {
    if (m_host) {
        m_host->LogInfo("SamplePlugin: Stopped");
    }
    return true;
}

void SamplePlugin::Destroy() {
    // 释放资源
}

PluginFramework::IPluginService* SamplePlugin::GetService(const std::string& serviceName) {
    // 示例：返回 nullptr，插件暂不提供服务
    return nullptr;
}

std::vector<PluginFramework::PluginUIPage> SamplePlugin::GetUIPages() {
    return {};
}

PluginFramework::QWidget* SamplePlugin::CreateUIPage(const std::string& pageId, PluginFramework::QWidget* parent) {
    return nullptr;
}

// 插件工厂函数
extern "C" {

PLUGIN_DECL PluginFramework::IPlugin* GetPluginFactory() {
    return new SamplePlugin();
}

PLUGIN_DECL const char* GetPluginAPIVersion() {
    return "1.0.0";
}

PLUGIN_DECL const PluginFramework::PluginMetadata* GetPluginMetadata() {
    static PluginFramework::PluginMetadata metadata = {
        "com.pluginframework.sample",
        "Sample Plugin",
        "1.0.0",
        "PluginFramework",
        "A sample plugin demonstrating the plugin framework",
        "1.0.0",
        {},
        "GetPluginFactory"
    };
    return &metadata;
}

} // extern "C"
