#include "JwtTool.h"
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <vector>
#include <sstream>

using json = nlohmann::json;

// Base64URL алфавит отличается от обычного Base64 символами '-' и '_' вместо '+' и '/'
namespace {
    std::string DecodeBase64Url(const std::string& input) {
        std::vector<int> table(256, -1);
        const std::string chars =
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz"
            "0123456789-_";
        for (int i = 0; i < 64; i++) table[(unsigned char)chars[i]] = i;

        std::string result;
        int val = 0, valb = -8;
        for (unsigned char c : input) {
            if (c == '=') break;               // На случай, если padding всё же есть
            if (table[c] == -1) continue;      // Пропускаем пробелы/переносы строк
            val = (val << 6) + table[c];
            valb += 6;
            if (valb >= 0) {
                result.push_back(char((val >> valb) & 0xFF));
                valb -= 8;
            }
        }
        return result;
    }

    // Разбивает строку "header.payload.signature" на три части
    std::vector<std::string> SplitJwt(const std::string& token) {
        std::vector<std::string> parts;
        std::stringstream ss(token);
        std::string part;
        while (std::getline(ss, part, '.')) {
            parts.push_back(part);
        }
        return parts;
    }
}

std::string JwtTool::Execute(const std::string& action, const std::string& payload) {
    if (action != "decode") {
        throw std::runtime_error("Unknown action: " + action);
    }

    auto parts = SplitJwt(payload);
    if (parts.size() != 3) {
        throw std::runtime_error("Invalid JWT: expected 3 parts separated by '.', got " + std::to_string(parts.size()));
    }

    std::string headerJson;
    std::string payloadJson;
    try {
        headerJson = DecodeBase64Url(parts[0]);
        payloadJson = DecodeBase64Url(parts[1]);

        // Проверяем, что декодированные части — валидный JSON
        json headerParsed = json::parse(headerJson);
        json payloadParsed = json::parse(payloadJson);

        json result = {
            {"header", headerParsed},
            {"payload", payloadParsed},
            {"signature", parts[2]}
        };

        return result.dump(4, ' ', false);
    }
    catch (const json::parse_error& e) {
        throw std::runtime_error(std::string("Invalid JWT: decoded content is not valid JSON (") + e.what() + ")");
    }
}