#pragma once
#include <windows.h>
#include "ITool.h"
#include <deque>
#include <string>
#include <mutex>

struct ClipboardEntry {
    std::string type;    // "text" или "image"
    std::string content; // текст, либо base64-encoded PNG
    int width = 0;
    int height = 0;
};

class ClipboardTool : public ITool {
public:
    std::string Execute(const std::string& action, const std::string& payload) override;
    void OnClipboardChanged(HWND hwnd);

private:
    void TrimType(const std::string& type, size_t limit);

    std::deque<ClipboardEntry> m_history;
    std::mutex m_mutex;
    static constexpr size_t kMaxTextEntries = 50;
    static constexpr size_t kMaxImageEntries = 10;
};