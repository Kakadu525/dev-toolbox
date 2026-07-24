cmake_minimum_required(VERSION 3.20)
project(DevToolbox LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT WIN32)
    message(FATAL_ERROR "Этот проект собирается только на Windows")
endif()

include(FetchContent)

FetchContent_Declare(
    webview2
    URL https://www.nuget.org/api/v2/package/Microsoft.Web.WebView2/1.0.2792.45
    DOWNLOAD_NAME webview2.zip
)
FetchContent_MakeAvailable(webview2)
set(WEBVIEW2_DIR ${webview2_SOURCE_DIR})

FetchContent_Declare(
    nlohmann_json
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
)
FetchContent_MakeAvailable(nlohmann_json)

FetchContent_Declare(
    pugixml
    GIT_REPOSITORY https://github.com/zeux/pugixml.git
    GIT_TAG v1.14
)
FetchContent_MakeAvailable(pugixml)

set(YAML_CPP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(YAML_CPP_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
set(YAML_CPP_BUILD_CONTRIB OFF CACHE BOOL "" FORCE)
set(YAML_BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
FetchContent_Declare(
    yaml-cpp
    GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
    GIT_TAG 0.8.0
)
FetchContent_MakeAvailable(yaml-cpp)
unset(CMAKE_POLICY_VERSION_MINIMUM)

add_executable(DevToolbox WIN32
    src/main.cpp
    src/WebViewHost.cpp
    src/WebViewHost.h
    src/ToolRegistry.cpp
    src/ToolRegistry.h
    src/StringUtil.h
    src/tools/ITool.h
    src/tools/Base64Tool.cpp
    src/tools/Base64Tool.h
    src/tools/HashTool.cpp
    src/tools/HashTool.h
    src/tools/UuidTool.cpp
    src/tools/UuidTool.h
    src/tools/JsonTool.cpp
    src/tools/JsonTool.h
    src/tools/XmlTool.cpp
    src/tools/XmlTool.h
    src/tools/YamlTool.cpp
    src/tools/YamlTool.h
    src/tools/JwtTool.cpp
    src/tools/JwtTool.h
    src/tools/JwtTool.cpp
    src/tools/JwtTool.h
    src/tools/RegexTool.cpp
    src/tools/RegexTool.h
    src/tools/RegexTool.cpp
    src/tools/RegexTool.h
    src/tools/DiffTool.cpp
    src/tools/DiffTool.h
    src/tools/DiffTool.cpp
    src/tools/DiffTool.h
    src/tools/ColorTool.cpp
    src/tools/ColorTool.h
    src/tools/ColorTool.cpp
    src/tools/ColorTool.h
    src/tools/CronTool.cpp
    src/tools/CronTool.h
    src/tools/CronTool.cpp
    src/tools/CronTool.h
    src/tools/SqlTool.cpp
    src/tools/SqlTool.h
    src/tools/SqlTool.cpp
    src/tools/SqlTool.h
    src/tools/CurlTool.cpp
    src/tools/CurlTool.h
)

target_include_directories(DevToolbox PRIVATE
    ${WEBVIEW2_DIR}/build/native/include
)

target_link_libraries(DevToolbox PRIVATE
    ${WEBVIEW2_DIR}/build/native/x64/WebView2LoaderStatic.lib
    nlohmann_json::nlohmann_json
    bcrypt.lib
    rpcrt4.lib
    pugixml
    yaml-cpp::yaml-cpp
)

target_compile_definitions(DevToolbox PRIVATE UNICODE _UNICODE)

add_custom_command(TARGET DevToolbox POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/ui
        $<TARGET_FILE_DIR:DevToolbox>/ui
)