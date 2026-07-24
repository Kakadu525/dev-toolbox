#pragma once
#include "ITool.h"
#include <functional>
#include <atomic>
#include <thread>
#include <string>

class LogTool : public ITool {
public:
    using PushCallback = std::function<void(const std::string&)>;

    explicit LogTool(PushCallback pushCallback);
    ~LogTool();

    std::string Execute(const std::string& action, const std::string& payload) override;

private:
    void StopWatching();
    void WatchLoop(std::string filePath, long long startOffset);

    PushCallback m_push;
    std::atomic<bool> m_watching{ false };
    std::thread m_watchThread;
};