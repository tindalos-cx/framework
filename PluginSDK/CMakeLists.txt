# PluginSDK - 插件开发接口库（纯头文件 INTERFACE 库）

add_library(PluginSDK INTERFACE)

target_include_directories(PluginSDK INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Qt6 依赖（用于 QWidget* 类型）
target_link_libraries(PluginSDK INTERFACE Qt6::Widgets)

# 导出为 PluginFramework::PluginSDK
add_library(PluginFramework::PluginSDK ALIAS PluginSDK)
