#include "ConfigManager.h"
#include <fstream>
#include <sstream>
#include <filesystem>

#ifdef _WIN32
    #include <shlobj.h>
#else
    #include <unistd.h>
    #include <sys/types.h>
    #include <pwd.h>
#endif

namespace fs = std::filesystem;

namespace PluginFramework {

ConfigManager::ConfigManager() = default;

ConfigManager::~ConfigManager() = default;

bool ConfigManager::Initialize() {
    m_configFilePath = GetDefaultConfigPath();
    EnsureConfigDirectory();

    if (fs::exists(m_configFilePath)) {
        return Load();
    }

    // 使用默认配置
    m_pluginDirectory = GetConfigDirectory() + "/plugins";
    return true;
}

bool ConfigManager::Load() {
    try {
        std::ifstream file(m_configFilePath);
        if (!file.is_open()) {
            return false;
        }

        std::string line;
        std::string currentSection;

        while (std::getline(file, line)) {
            // 去除空白字符
            line.erase(0, line.find_first_not_of(" \t"));
            line.erase(line.find_last_not_of(" \t") + 1);

            if (line.empty() || line[0] == '#' || line[0] == ';') {
                continue;
            }

            if (line[0] == '[' && line.back() == ']') {
                currentSection = line.substr(1, line.length() - 2);
                continue;
            }

            size_t pos = line.find('=');
            if (pos == std::string::npos) continue;

            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);

            // 去除引号
            if (!value.empty() && value.front() == '"' && value.back() == '"') {
                value = value.substr(1, value.length() - 2);
            }

            if (currentSection == "General") {
                if (key == "PluginDirectory") {
                    m_pluginDirectory = value;
                }
            } else if (currentSection == "EnabledPlugins") {
                if (!key.empty()) {
                    m_enabledPlugins.push_back(key);
                }
            } else if (currentSection == "Settings") {
                m_config[key] = value;
            }
        }

        return true;
    } catch (const std::exception& e) {
        return false;
    }
}

bool ConfigManager::Save() {
    try {
        std::ofstream file(m_configFilePath);
        if (!file.is_open()) {
            return false;
        }

        // 写入配置
        file << "[General]\n";
        file << "PluginDirectory=" << m_pluginDirectory << "\n\n";

        file << "[EnabledPlugins]\n";
        for (const auto& pluginId : m_enabledPlugins) {
            file << pluginId << "\n";
        }
        file << "\n";

        file << "[Settings]\n";
        for (const auto& pair : m_config) {
            file << pair.first << "=" << pair.second << "\n";
        }

        return true;
    } catch (const std::exception& e) {
        return false;
    }
}

std::vector<std::string> ConfigManager::GetEnabledPlugins() const {
    return m_enabledPlugins;
}

void ConfigManager::SetEnabledPlugins(const std::vector<std::string>& plugins) {
    m_enabledPlugins = plugins;
}

bool ConfigManager::IsPluginEnabled(const std::string& pluginId) const {
    return std::find(m_enabledPlugins.begin(), m_enabledPlugins.end(), pluginId) != m_enabledPlugins.end();
}

void ConfigManager::SetPluginEnabled(const std::string& pluginId, bool enabled) {
    auto it = std::find(m_enabledPlugins.begin(), m_enabledPlugins.end(), pluginId);

    if (enabled) {
        if (it == m_enabledPlugins.end()) {
            m_enabledPlugins.push_back(pluginId);
        }
    } else {
        if (it != m_enabledPlugins.end()) {
            m_enabledPlugins.erase(it);
        }
    }
}

std::string ConfigManager::GetString(const std::string& key, const std::string& defaultValue) const {
    auto it = m_config.find(key);
    return it != m_config.end() ? it->second : defaultValue;
}

int ConfigManager::GetInt(const std::string& key, int defaultValue) const {
    auto it = m_config.find(key);
    if (it != m_config.end()) {
        try {
            return std::stoi(it->second);
        } catch (...) {}
    }
    return defaultValue;
}

bool ConfigManager::GetBool(const std::string& key, bool defaultValue) const {
    auto it = m_config.find(key);
    if (it != m_config.end()) {
        const auto& val = it->second;
        return val == "true" || val == "1" || val == "yes";
    }
    return defaultValue;
}

void ConfigManager::SetString(const std::string& key, const std::string& value) {
    m_config[key] = value;
}

void ConfigManager::SetInt(const std::string& key, int value) {
    m_config[key] = std::to_string(value);
}

void ConfigManager::SetBool(const std::string& key, bool value) {
    m_config[key] = value ? "true" : "false";
}

std::string ConfigManager::GetPluginDirectory() const {
    return m_pluginDirectory;
}

void ConfigManager::SetPluginDirectory(const std::string& path) {
    m_pluginDirectory = path;
}

std::string ConfigManager::GetConfigDirectory() const {
    return GetDefaultConfigPath().substr(0, GetDefaultConfigPath().rfind('/'));
}

void ConfigManager::EnsureConfigDirectory() {
    auto dir = GetConfigDirectory();
    if (!fs::exists(dir)) {
        fs::create_directories(dir);
    }
}

std::string ConfigManager::GetDefaultConfigPath() {
#ifdef _WIN32
    char appData[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, 0, appData))) {
        return std::string(appData) + "/PluginFramework/plugins.json";
    }
    return "plugins.json";
#elif __APPLE__
    auto home = getenv("HOME");
    if (home) {
        return std::string(home) + "/Library/Application Support/PluginFramework/plugins.json";
    }
    return "plugins.json";
#else
    auto home = getenv("HOME");
    if (home) {
        return std::string(home) + "/.config/PluginFramework/plugins.json";
    }
    return "plugins.json";
#endif
}

} // namespace PluginFramework
