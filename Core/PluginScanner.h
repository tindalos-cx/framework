#pragma once

#include "PluginTypes.h"
#include <string>
#include <vector>

namespace PluginFramework {

// 插件扫描器
// 负责扫描目录中的插件并读取元数据
class PluginScanner {
public:
    PluginScanner();
    ~PluginScanner();

    // 扫描目录中的所有插件
    std::vector<PluginMetadata> Scan(const std::string& directory);

    // 扫描并返回插件文件路径
    std::vector<std::string> ScanFiles(const std::string& directory);

    // 读取单个插件的元数据
    PluginMetadata ReadMetadata(const std::string& filePath);

    // 验证插件文件是否有效
    bool ValidatePlugin(const std::string& filePath);

private:
    std::string GetPluginExtension();
    bool IsValidPluginFile(const std::string& filePath);
};

} // namespace PluginFramework
