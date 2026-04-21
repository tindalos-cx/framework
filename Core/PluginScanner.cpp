#include "PluginScanner.h"
#include <filesystem>
#include <fstream>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <dlfcn.h>
#endif

namespace fs = std::filesystem;

namespace PluginFramework {

PluginScanner::PluginScanner() = default;

PluginScanner::~PluginScanner() = default;

std::vector<PluginMetadata> PluginScanner::Scan(const std::string& directory) {
    std::vector<PluginMetadata> result;

    try {
        if (!fs::exists(directory) || !fs::is_directory(directory)) {
            return result;
        }

        for (const auto& entry : fs::directory_iterator(directory)) {
            if (!entry.is_regular_file()) continue;

            const auto& path = entry.path();
            if (!IsValidPluginFile(path.string())) continue;

            auto metadata = ReadMetadata(path.string());
            if (!metadata.id.empty()) {
                result.push_back(std::move(metadata));
            }
        }
    } catch (const fs::filesystem_error& e) {
        // 记录错误日志
    }

    return result;
}

std::vector<std::string> PluginScanner::ScanFiles(const std::string& directory) {
    std::vector<std::string> result;

    try {
        if (!fs::exists(directory) || !fs::is_directory(directory)) {
            return result;
        }

        for (const auto& entry : fs::directory_iterator(directory)) {
            if (!entry.is_regular_file()) continue;

            const auto& path = entry.path();
            if (IsValidPluginFile(path.string())) {
                result.push_back(path.string());
            }
        }
    } catch (const fs::filesystem_error& e) {
        // 记录错误日志
    }

    return result;
}

PluginMetadata PluginScanner::ReadMetadata(const std::string& filePath) {
    PluginMetadata metadata;

    // 尝试从嵌入的元数据文件读取
    // 约定：插件目录下需要有同名 .json 文件
    std::string jsonPath = filePath;
    size_t extPos = jsonPath.rfind('.');
    if (extPos != std::string::npos) {
        jsonPath = jsonPath.substr(0, extPos);
    }
    jsonPath += ".json";

    if (fs::exists(jsonPath)) {
        // 简化处理：直接读取 JSON 文件（实际应使用 JSON 库解析）
        std::ifstream file(jsonPath);
        if (file.is_open()) {
            std::string content((std::istreambuf_iterator<char>(file)),
                                std::istreambuf_iterator<char>());
            // TODO: 使用 nlohmann/json 解析
            // 这里简化处理，从文件名提取 ID
            fs::path p(filePath);
            metadata.id = p.stem().string();
            metadata.name = metadata.id;
        }
    }

    // 如果无法读取，返回基本信息
    if (metadata.id.empty()) {
        fs::path p(filePath);
        metadata.id = p.stem().string();
        metadata.name = metadata.id;
    }

    return metadata;
}

bool PluginScanner::ValidatePlugin(const std::string& filePath) {
    if (!IsValidPluginFile(filePath)) {
        return false;
    }

    // 尝试加载模块并检查必要的导出函数
#ifdef _WIN32
    HMODULE module = LoadLibraryA(filePath.c_str());
    if (!module) return false;

    auto factory = GetProcAddress(module, "GetPluginFactory");
    auto metadata = GetProcAddress(module, "GetPluginMetadata");

    FreeLibrary(module);
    return factory != nullptr && metadata != nullptr;
#else
    void* module = dlopen(filePath.c_str(), RTLD_NOLOAD | RTLD_LAZY);
    if (!module) return false;

    auto factory = dlsym(module, "GetPluginFactory");
    auto metadata = dlsym(module, "GetPluginMetadata");

    dlclose(module);
    return factory != nullptr && metadata != nullptr;
#endif
}

std::string PluginScanner::GetPluginExtension() {
#ifdef _WIN32
    return ".dll";
#elif __APPLE__
    return ".dylib";
#else
    return ".so";
#endif
}

bool PluginScanner::IsValidPluginFile(const std::string& filePath) {
    fs::path p(filePath);
    std::string ext = p.extension().string();

    std::string expectedExt = GetPluginExtension();

    // 大小写不敏感比较
    if (ext.length() != expectedExt.length()) return false;

    for (size_t i = 0; i < ext.length(); ++i) {
        if (tolower(ext[i]) != tolower(expectedExt[i])) {
            return false;
        }
    }

    return true;
}

} // namespace PluginFramework
