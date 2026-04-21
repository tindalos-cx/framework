# SamplePlugin - 示例插件

set(SAMPLE_PLUGIN_SOURCES
    SamplePlugin.h
    SamplePlugin.cpp
)

add_library(SamplePlugin SHARED ${SAMPLE_PLUGIN_SOURCES})

target_link_libraries(SamplePlugin PRIVATE
    PluginFramework::PluginSDK
)

target_include_directories(SamplePlugin PRIVATE
    ${CMAKE_SOURCE_DIR}/PluginSDK/include
)

# 设置输出目录
set_target_properties(SamplePlugin PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${PLUGIN_OUTPUT_DIR}
    OUTPUT_NAME "SamplePlugin"
)

# Windows: 设置 DLL 输出目录
if(WIN32)
    set_target_properties(SamplePlugin PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${PLUGIN_OUTPUT_DIR}
    )
endif()

# 标记为插件
target_compile_definitions(SamplePlugin PRIVATE PLUGIN_EXPORTS)
