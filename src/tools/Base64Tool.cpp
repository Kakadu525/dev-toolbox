#include "Base64Tool.h"
#include <stdexcept>
#include <vector>

namespace {
    const std::string kChars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";

    std::string Encode(const std::string& input) {
        std::string result;
        int val = 0, valb = -6;
        for (unsigned char c : input) {
            val = (val << 8) + c;
            valb += 8;
            while (valb >= 0) {
                result.push_back(kChars[(val >> valb) & 0x3F]);
                valb -= 6;
            }
        }
        if (valb > -6) result.push_back(kChars[((val << 8) >> (valb + 8)) & 0x3F]);
        while (result.size() % 4) result.push_back('=');
        return result;
    }

    std::string Decode(const std::string& input) {
        std::vector<int> table(256, -1);
        for (int i = 0; i < 64; i++) table[(unsigned char)kChars[i]] = i;

        std::string result;
        int val = 0, valb = -8;
        for (unsigned char c : input) {
            if (c == '=') break;
            if (table[c] == -1) continue;
            val = (val << 6) + table[c];
            valb += 6;
            if (valb >= 0) {
                result.push_back(char((val >> valb) & 0xFF));
                valb -= 8;
            }
        }
        return result;
    }
}

std::string Base64Tool::Execute(const std::string& action, const std::string& payload) {
    if (action == "encode") return Encode(payload);
    if (action == "decode") return Decode(payload);
    throw std::runtime_error("Unknown action: " + action);
}