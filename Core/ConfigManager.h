#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>

namespace PluginFramework {

// 配置管理器
// 负责读取和保存插件配置文件
class ConfigManager {
public:
    ConfigManager();
    ~ConfigManager();

    // 初始化
    bool Initialize();
    bool Load();
    bool Save();

    // 插件配置
    std::vector<std::string> GetEnabledPlugins() const;
    void SetEnabledPlugins(const std::vector<std::string>& plugins);

    bool IsPluginEnabled(const std::string& pluginId) const;
    void SetPluginEnabled(const std::string& pluginId, bool enabled);

    // 通用配置
    std::string GetString(const std::string& key, const std::string& defaultValue = "") const;
    int GetInt(const std::string& key, int defaultValue = 0) const;
    bool GetBool(const std::string& key, bool defaultValue = false) const;

    void SetString(const std::string& key, const std::string& value);
    void SetInt(const std::string& key, int value);
    void SetBool(const std::string& key, bool value);

    // 路径配置
    std::string GetPluginDirectory() const;
    void SetPluginDirectory(const std::string& path);

    std::string GetConfigDirectory() const;

private:
    std::string m_configFilePath;
    std::map<std::string, std::string> m_config;
    std::vector<std::string> m_enabledPlugins;
    std::string m_pluginDirectory;

    void EnsureConfigDirectory();
    std::string GetDefaultConfigPath();
};

} // namespace PluginFramework
