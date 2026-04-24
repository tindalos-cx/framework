#pragma once

#include <IPlugin.h>
#include <string>

class SamplePlugin : public PluginFramework::IPlugin {
public:
    SamplePlugin();
    ~SamplePlugin() override;

    // IPlugin 接口实现
    PluginFramework::PluginMetadata GetMetadata() const override;
    bool Initialize(PluginFramework::IHost* host) override;
    bool Start() override;
    bool Stop() override;
    void Destroy() override;

    PluginFramework::IPluginService* GetService(const std::string& serviceName) override;
    std::vector<PluginFramework::PluginUIPage> GetUIPages() override;
    QWidget* CreateUIPage(const std::string& pageId, QWidget* parent) override;

private:
    PluginFramework::PluginMetadata m_metadata;
    PluginFramework::IHost* m_host;
};
