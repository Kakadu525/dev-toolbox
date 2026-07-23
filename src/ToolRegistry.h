#pragma once
#include <string>
#include <unordered_map>
#include <memory>
#include "tools/ITool.h"

class ToolRegistry {
public:
    static ToolRegistry& Instance();
    void Register(const std::string& name, std::unique_ptr<ITool> tool);
    std::string Handle(const std::string& jsonMessage);

private:
    std::unordered_map<std::string, std::unique_ptr<ITool>> m_tools;
};