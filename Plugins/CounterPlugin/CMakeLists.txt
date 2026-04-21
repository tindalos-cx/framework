# CounterPlugin - 计数器插件示例

set(COUNTER_PLUGIN_SOURCES
    CounterPlugin.h
    CounterPlugin.cpp
)

add_library(CounterPlugin SHARED ${COUNTER_PLUGIN_SOURCES})

target_link_libraries(CounterPlugin PRIVATE
    PluginFramework::PluginSDK
)

target_include_directories(CounterPlugin PRIVATE
    ${CMAKE_SOURCE_DIR}/PluginSDK/include
)

# 设置输出目录
set_target_properties(CounterPlugin PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${PLUGIN_OUTPUT_DIR}
    OUTPUT_NAME "CounterPlugin"
)

# Windows: 设置 DLL 输出目录
if(WIN32)
    set_target_properties(CounterPlugin PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${PLUGIN_OUTPUT_DIR}
    )
endif()

# 标记为插件
target_compile_definitions(CounterPlugin PRIVATE PLUGIN_EXPORTS)
