cmake_minimum_required(VERSION 3.20)
project(DevToolbox LANGUAGES CXX C)

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

FetchContent_Declare(
    qrcodegen
    GIT_REPOSITORY https://github.com/nayuki/QR-Code-generator.git
    GIT_TAG v1.8.0
)
FetchContent_MakeAvailable(qrcodegen)

FetchContent_Declare(
    stb
    GIT_REPOSITORY https://github.com/nothings/stb.git
    GIT_TAG master
)
FetchContent_MakeAvailable(stb)

set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
FetchContent_Declare(
    miniz
    GIT_REPOSITORY https://github.com/richgel999/miniz.git
    GIT_TAG 3.0.2
)
FetchContent_MakeAvailable(miniz)
unset(CMAKE_POLICY_VERSION_MINIMUM)

# ── Упаковка ui/ в один zip-архив на этапе сборки, чтобы встроить его в exe ──
set(UI_ZIP_PATH ${CMAKE_BINARY_DIR}/ui_embedded.zip)
file(GLOB_RECURSE UI_SOURCE_FILES CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/ui/*)

add_custom_command(
    OUTPUT ${UI_ZIP_PATH}
    COMMAND ${CMAKE_COMMAND} -E tar cf ${UI_ZIP_PATH} --format=zip -- .
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/ui
    DEPENDS ${UI_SOURCE_FILES}
    COMMENT "Packaging ui/ into ui_embedded.zip for embedding into the exe"
)
add_custom_target(ui_zip_target DEPENDS ${UI_ZIP_PATH})

configure_file(
    ${CMAKE_SOURCE_DIR}/resources/ui_data.rc.in
    ${CMAKE_BINARY_DIR}/ui_data.rc
    @ONLY
)

add_executable(DevToolbox WIN32
    src/main.cpp
    src/WebViewHost.cpp
    src/WebViewHost.h
    src/ToolRegistry.cpp
    src/ToolRegistry.h
    src/StringUtil.h
    src/StbImpl.cpp
    src/EmbeddedUi.cpp
    src/EmbeddedUi.h
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
    src/tools/RegexTool.cpp
    src/tools/RegexTool.h
    src/tools/DiffTool.cpp
    src/tools/DiffTool.h
    src/tools/ColorTool.cpp
    src/tools/ColorTool.h
    src/tools/CronTool.cpp
    src/tools/CronTool.h
    src/tools/SqlTool.cpp
    src/tools/SqlTool.h
    src/tools/CurlTool.cpp
    src/tools/CurlTool.h
    src/tools/QrTool.cpp
    src/tools/QrTool.h
    ${qrcodegen_SOURCE_DIR}/cpp/qrcodegen.cpp
    src/tools/HttpTool.cpp
    src/tools/HttpTool.h
    src/tools/ImageTool.cpp
    src/tools/ImageTool.h
    src/tools/LogTool.cpp
    src/tools/LogTool.h
    src/tools/ProcessTool.cpp
    src/tools/ProcessTool.h
    src/tools/ClipboardTool.cpp
    src/tools/ClipboardTool.h
    src/tools/SettingsTool.cpp
    src/tools/SettingsTool.h
    resources/app.rc
    ${CMAKE_BINARY_DIR}/ui_data.rc
)

add_dependencies(DevToolbox ui_zip_target)

target_include_directories(DevToolbox PRIVATE
    ${WEBVIEW2_DIR}/build/native/include
    ${qrcodegen_SOURCE_DIR}/cpp
    ${stb_SOURCE_DIR}
    ${miniz_SOURCE_DIR}
    ${miniz_BINARY_DIR}
)

target_link_libraries(DevToolbox PRIVATE
    ${WEBVIEW2_DIR}/build/native/x64/WebView2LoaderStatic.lib
    nlohmann_json::nlohmann_json
    bcrypt.lib
    rpcrt4.lib
    pugixml
    yaml-cpp::yaml-cpp
    winhttp.lib
    comdlg32.lib
    psapi.lib
    gdi32.lib
    advapi32.lib
    miniz
)

target_compile_definitions(DevToolbox PRIVATE UNICODE _UNICODE)

set_target_properties(DevToolbox PROPERTIES
    LINK_FLAGS "/MANIFESTUAC:\"level='requireAdministrator' uiAccess='false'\""
)

# Копируем ui/ рядом с exe как запасной вариант (fallback), на случай если
# извлечение встроенного ресурса в рантайме почему-то не сработает
add_custom_command(TARGET DevToolbox POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/ui
        $<TARGET_FILE_DIR:DevToolbox>/ui
)