#include "YamlTool.h"
#include <yaml-cpp/yaml.h>
#include <stdexcept>

std::string YamlTool::Execute(const std::string& action, const std::string& payload) {
    if (action != "format") {
        throw std::runtime_error("Unknown action: " + action);
    }

    YAML::Node parsed;
    try {
        parsed = YAML::Load(payload);
    }

    catch (const YAML::ParserException& e) {
        throw std::runtime_error(std::string("Invalid YAML: ") + e.what());
    }

    YAML::Emitter emitter;
    emitter << parsed;

    return std::string(emitter.c_str());
}