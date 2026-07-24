#include "LogTool.h"
#include <windows.h>
#include <commdlg.h>
#include <nlohmann/json.hpp>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include "../StringUtil.h"
#pragma comment(lib, "comdlg32.lib")

using json = nlohmann::json;

LogTool::LogTool(PushCallback pushCallback) : m_push(std::move(pushCallback)) {}

LogTool::~LogTool() {
    StopWatching();
}

void LogTool::StopWatching() {
    if (m_watching) {
        m_watching = false;
        if (m_watchThread.joinable()) m_watchThread.join();
    }
}

std::string LogTool::Execute(const std::string& action, const std::string& payload) {
    if (action == "pickFile") {
        wchar_t fileBuf[MAX_PATH] = {};
        OPENFILENAMEW ofn = {};
        ofn.lStructSize = sizeof(ofn);
        ofn.lpstrFilter = L"Log files (*.log;*.txt)\0*.log;*.txt\0All files\0*.*\0";
        ofn.lpstrFile = fileBuf;
        ofn.nMaxFile = MAX_PATH;
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

        if (!GetOpenFileNameW(&ofn)) {
            json result = { {"cancelled", true} };
            return result.dump();
        }

        std::string path = WideToUtf8(fileBuf);
        json result = { {"path", path} };
        return result.dump();
    }

    if (action == "readAll") {
        std::string path = payload;
        std::ifstream file(path, std::ios::binary);
        if (!file) throw std::runtime_error("Cannot open file: " + path);

        std::ostringstream ss;
        ss << file.rdbuf();
        std::string content = ss.str();

        json result = { {"content", content}, {"sizeBytes", (long long)content.size()} };
        return result.dump();
    }

    if (action == "startWatch") {
        StopWatching();

        json request = json::parse(payload);
        std::string path = request.value("path", "");
        long long startOffset = request.value("offset", 0LL);

        if (path.empty()) throw std::runtime_error("Path is required");

        m_watching = true;
        m_watchThread = std::thread(&LogTool::WatchLoop, this, path, startOffset);

        json result = { {"status", "watching"} };
        return result.dump();
    }

    if (action == "stopWatch") {
        StopWatching();
        json result = { {"status", "stopped"} };
        return result.dump();
    }

    throw std::runtime_error("Unknown action: " + action);
}

// Проверяем файл на новые данные раз в секунду
void LogTool::WatchLoop(std::string filePath, long long lastOffset) {
    while (m_watching) {
        std::ifstream file(filePath, std::ios::binary);
        if (file) {
            file.seekg(0, std::ios::end);
            long long currentSize = file.tellg();

            if (currentSize > lastOffset) {
                file.seekg(lastOffset);
                std::ostringstream ss;
                ss << file.rdbuf();
                std::string newContent = ss.str();

                json notification = {
                    {"tool", "log"},
                    {"event", "newLines"},
                    {"content", newContent}
                };
                m_push(notification.dump());

                lastOffset = currentSize;
            }
            else if (currentSize < lastOffset) {
                lastOffset = 0;
            }
        }

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}