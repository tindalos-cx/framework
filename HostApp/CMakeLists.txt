# HostApp - 宿主应用

set(HOSTAPP_SOURCES
    main.cpp
    MainWindow.cpp
    PluginManagerDialog.cpp
)

add_executable(PluginHostApp ${HOSTAPP_SOURCES})

# Qt6 依赖
target_link_libraries(PluginHostApp PRIVATE Qt6::Core Qt6::Widgets)

target_link_libraries(PluginHostApp PRIVATE
    PluginFramework::PluginCore
    PluginFramework::PluginSDK
)

target_include_directories(PluginHostApp PRIVATE
    ${CMAKE_SOURCE_DIR}/Core
)

set_target_properties(PluginHostApp PROPERTIES
    OUTPUT_NAME "PluginHostApp"
)

# 复制插件到输出目录的后期处理
add_custom_command(TARGET PluginHostApp POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/Plugins $<TARGET_FILE_DIR:PluginHostApp>/plugins
)
