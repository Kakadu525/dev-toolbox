#include "SettingsTool.h"
#include "../StringUtil.h"
#include <windows.h>
#include <shlwapi.h>
#include <nlohmann/json.hpp>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <shlobj.h>
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "shlwapi.lib")

using json = nlohmann::json;

std::string SettingsTool::GetSettingsFilePath() {
    wchar_t appDataPath[MAX_PATH];
    if (FAILED(SHGetFolderPathW(nullptr, CSIDL_APPDATA, nullptr, 0, appDataPath))) {
        GetTempPathW(MAX_PATH, appDataPath);
    }

    std::wstring dir = std::wstring(appDataPath) + L"\\DevToolbox";
    CreateDirectoryW(dir.c_str(), nullptr); // если уже существует — ошибка просто игнорируется

    std::wstring path = dir + L"\\settings.json";
    return WideToUtf8(path);
}

std::string SettingsTool::Execute(const std::string& action, const std::string& payload) {
    if (action == "load") {
        std::string path = GetSettingsFilePath();
        std::ifstream file(path);

        json defaults = {
            {"theme", "dark"},
            {"fontSize", "medium"},
            {"accentColor", "#06b6d4"}
        };

        if (!file) {
            return defaults.dump();
        }

        try {
            std::ostringstream ss;
            ss << file.rdbuf();
            json loaded = json::parse(ss.str());
            for (auto& [key, value] : defaults.items()) {
                if (!loaded.contains(key)) loaded[key] = value;
            }
            return loaded.dump();
        }
        catch (...) {
            return defaults.dump();
        }
    }

    if (action == "save") {
        json settings;
        try {
            settings = json::parse(payload);
        }
        catch (const json::parse_error&) {
            throw std::runtime_error("Invalid settings format");
        }

        std::string path = GetSettingsFilePath();
        std::ofstream file(path);
        if (!file) {
            throw std::runtime_error("Failed to write settings file");
        }
        file << settings.dump(4);

        json result = { {"saved", true} };
        return result.dump();
    }

    throw std::runtime_error("Unknown action: " + action);
}