#pragma once
#include <string>
#include <unordered_map>
#include <memory>
#include <functional>
#include "tools/ITool.h"

class ToolRegistry {
public:
    static ToolRegistry& Instance();
    void Register(const std::string& name, std::unique_ptr<ITool> tool);
    std::string Handle(const std::string& jsonMessage);

    // Callback, через который инструменты могут отправлять сообщения в UI
    void SetPushCallback(std::function<void(const std::string&)> callback);
    std::function<void(const std::string&)> GetPushCallback() const;

private:
    std::unordered_map<std::string, std::unique_ptr<ITool>> m_tools;
    std::function<void(const std::string&)> m_pushCallback;
};