#include "RegexTool.h"
#include <nlohmann/json.hpp>
#include <regex>
#include <stdexcept>

using json = nlohmann::json;

std::string RegexTool::Execute(const std::string& action, const std::string& payload) {
    if (action != "test") {
        throw std::runtime_error("Unknown action: " + action);
    }

    json request;
    try {
        request = json::parse(payload);
    }
    catch (const json::parse_error&) {
        throw std::runtime_error("Invalid request format");
    }

    std::string pattern = request.value("pattern", "");
    std::string text = request.value("text", "");

    std::regex re;
    try {
        re = std::regex(pattern, std::regex::ECMAScript);
    }
    catch (const std::regex_error& e) {
        throw std::runtime_error(std::string("Invalid regex pattern: ") + e.what());
    }

    json matches = json::array();
    auto begin = std::sregex_iterator(text.begin(), text.end(), re);
    auto end = std::sregex_iterator();

    for (auto it = begin; it != end; ++it) {
        std::smatch match = *it;
        json matchObj = {
            {"match", match.str()},
            {"position", (int)match.position()}
        };

        if (match.size() > 1) {
            json groups = json::array();
            for (size_t i = 1; i < match.size(); i++) {
                groups.push_back(match[i].matched ? match[i].str() : "");
            }
            matchObj["groups"] = groups;
        }

        matches.push_back(matchObj);
    }

    json result = {
        {"matchCount", (int)matches.size()},
        {"matches", matches}
    };

    return result.dump(4, ' ', false);
}