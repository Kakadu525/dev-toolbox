#include "CurlTool.h"
#include <nlohmann/json.hpp>
#include <sstream>
#include <stdexcept>

using json = nlohmann::json;

// Оборачивает значение в одинарные кавычки
namespace {
    std::string ShellQuote(const std::string& value) {
        std::string result = "'";
        for (char c : value) {
            if (c == '\'') {
                result += "'\\''";
            }
            else {
                result += c;
            }
        }
        result += "'";
        return result;
    }
}

std::string CurlTool::Execute(const std::string& action, const std::string& payload) {
    if (action != "generate") {
        throw std::runtime_error("Unknown action: " + action);
    }

    json request;
    try {
        request = json::parse(payload);
    }
    catch (const json::parse_error&) {
        throw std::runtime_error("Invalid request format");
    }

    std::string method = request.value("method", "GET");
    std::string url = request.value("url", "");
    std::string body = request.value("body", "");
    json headers = request.value("headers", json::array());

    if (url.empty()) {
        throw std::runtime_error("URL is required");
    }

    std::ostringstream cmd;
    cmd << "curl -X " << method << " " << ShellQuote(url);

    for (const auto& header : headers) {
        std::string key = header.value("key", "");
        std::string value = header.value("value", "");
        if (key.empty()) continue;
        cmd << " \\\n  -H " << ShellQuote(key + ": " + value);
    }

    if (!body.empty()) {
        cmd << " \\\n  -d " << ShellQuote(body);
    }

    return cmd.str();
}