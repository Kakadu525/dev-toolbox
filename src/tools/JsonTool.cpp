#include "JsonTool.h"
#include <nlohmann/json.hpp>
#include <stdexcept>

using json = nlohmann::json;

std::string JsonTool::Execute(const std::string& action, const std::string& payload) {
    json parsed;
    try {
        parsed = json::parse(payload);
    }
	// Отдаём понятную ошибку
    catch (const json::parse_error& e) {
        throw std::runtime_error(std::string("Invalid JSON: ") + e.what());
    }

    if (action == "pretty") {
        return parsed.dump(4, ' ', false);
    }
    if (action == "minify") {
        return parsed.dump(-1, ' ', false);
    }

    throw std::runtime_error("Unknown action: " + action);
}