# Core - 核心框架静态库

set(CORE_SOURCES
    PluginManager.cpp
    PluginRegistry.cpp
    EventBus.cpp
    PluginScanner.cpp
    ConfigManager.cpp
    HostAppAdapter.cpp
)

add_library(PluginCore STATIC ${CORE_SOURCES})

# Qt5 依赖
target_link_libraries(PluginCore PRIVATE Qt5::Core Qt5::Widgets)

# nlohmann/json 依赖
target_link_libraries(PluginCore PRIVATE nlohmann_json::nlohmann_json)

target_include_directories(PluginCore PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/PluginSDK/include>
    $<INSTALL_INTERFACE:include>
)

# 导出为 PluginFramework::PluginCore
add_library(PluginFramework::PluginCore ALIAS PluginCore)
