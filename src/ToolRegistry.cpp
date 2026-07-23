#include "ToolRegistry.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

ToolRegistry& ToolRegistry::Instance() {
    static ToolRegistry instance;
    return instance;
}

void ToolRegistry::Register(const std::string& name, std::unique_ptr<ITool> tool) {
    m_tools[name] = std::move(tool);
}

std::string ToolRegistry::Handle(const std::string& jsonMessage) {
    std::string toolName;
    try {
        json request = json::parse(jsonMessage);
        toolName = request.value("tool", "");
        std::string action = request.value("action", "");
        std::string payload = request.value("payload", "");

        auto it = m_tools.find(toolName);
        if (it == m_tools.end()) {
            json response = { {"tool", toolName}, {"status", "error"},
                               {"message", "Unknown tool: " + toolName} };
            return response.dump();
        }

        std::string result = it->second->Execute(action, payload);
        json response = { {"tool", toolName}, {"status", "ok"}, {"result", result} };
        return response.dump();
    }
    catch (const std::exception& e) {
        json response = { {"tool", toolName}, {"status", "error"}, {"message", std::string(e.what())} };
        return response.dump();
    }
}