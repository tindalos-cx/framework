#pragma once

#include <IPlugin.h>
#include <string>

class CounterPlugin : public PluginFramework::IPlugin {
public:
    CounterPlugin();
    ~CounterPlugin() override;

    // IPlugin 接口实现
    PluginFramework::PluginMetadata GetMetadata() const override;
    bool Initialize(PluginFramework::IHost* host) override;
    bool Start() override;
    bool Stop() override;
    void Destroy() override;

    PluginFramework::IPluginService* GetService(const std::string& serviceName) override;
    std::vector<PluginFramework::PluginUIPage> GetUIPages() override;
    PluginFramework::QWidget* CreateUIPage(const std::string& pageId, PluginFramework::QWidget* parent) override;

    // 计数器功能
    void Increment();
    void Decrement();
    int GetCount() const;
    void Reset();

private:
    PluginFramework::PluginMetadata m_metadata;
    PluginFramework::IHost* m_host;
    int m_counter;
};
