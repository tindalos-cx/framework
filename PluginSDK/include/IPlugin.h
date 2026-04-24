#pragma once

#include "PluginTypes.h"
#include "IHost.h"
#include "IPluginService.h"
#include <QWidget>
#include <vector>

// 插件导出宏
#ifdef PLUGIN_EXPORTS
    #define PLUGIN_DECL __declspec(dllexport)
#else
    #define PLUGIN_DECL __declspec(dllimport)
#endif

// SDK 导出宏（用于 PluginSDK 头文件）
#ifndef PLUGINSdk_API
    #ifdef PLUGINSdk_EXPORTS
        #define PLUGINSdk_API __declspec(dllexport)
    #else
        #define PLUGINSdk_API
    #endif
#endif

namespace PluginFramework {

// 插件基类接口
// 所有插件必须继承并实现此接口
class IPlugin {
public:
    virtual ~IPlugin() = default;

    // 获取插件元数据
    virtual PluginMetadata GetMetadata() const = 0;

    // 初始化插件（在加载后调用）
    // 返回 true 表示初始化成功
    virtual bool Initialize(IHost* host) = 0;

    // 启动插件（在所有依赖插件初始化后调用）
    virtual bool Start() = 0;

    // 停止插件（在卸载前调用）
    virtual bool Stop() = 0;

    // 销毁插件（释放资源）
    virtual void Destroy() = 0;

    // 获取插件提供的服务
    virtual IPluginService* GetService(const std::string& serviceName) { return nullptr; }

    // 获取插件的 UI 页面列表
    virtual std::vector<PluginUIPage> GetUIPages() { return {}; }

    // 创建 UI 页面
    virtual QWidget* CreateUIPage(const std::string& pageId, QWidget* parent = nullptr) { return nullptr; }
};

// 插件工厂函数指针类型
using PluginFactoryFunc = IPlugin* (*)();

} // namespace PluginFramework

// C 风格的插件入口函数（由插件 DLL 导出）
// 宿主通过此函数创建插件实例
// 注意：这些函数在全局命名空间中定义，不在 PluginFramework 命名空间内
extern "C" {
    // 获取插件工厂函数
    // 返回插件类的创建函数
    PLUGIN_DECL PluginFramework::IPlugin* GetPluginFactory();

    // 获取插件 API 版本
    // 用于检查兼容性
    PLUGIN_DECL const char* GetPluginAPIVersion();

    // 获取插件元数据
    PLUGIN_DECL const PluginFramework::PluginMetadata* GetPluginMetadata();
}
