cmake_minimum_required(VERSION 3.16)
project(PluginFramework VERSION 1.0.0 LANGUAGES CXX)

# C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Qt 自动处理
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# 输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# 寻找 Qt5 (优先 5.15)
set(QT_VERSION_MAJOR 5)
set(QT_MIN_VERSION 5.15)
find_package(Qt5 ${QT_MIN_VERSION} COMPONENTS Core Widgets REQUIRED)

# 插件输出目录
set(PLUGIN_OUTPUT_DIR ${CMAKE_BINARY_DIR}/bin/plugins)

# nlohmann/json via FetchContent
include(FetchContent)
FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(json)

# 包含子目录
include_directories(
    ${CMAKE_SOURCE_DIR}/Core
    ${CMAKE_SOURCE_DIR}/PluginSDK/include
)

add_subdirectory(PluginSDK)
add_subdirectory(Core)
add_subdirectory(HostApp)
add_subdirectory(Plugins)

# 创建插件目录
file(MAKE_DIRECTORY ${PLUGIN_OUTPUT_DIR})
