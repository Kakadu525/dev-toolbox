#include "ColorTool.h"
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cmath>

using json = nlohmann::json;

namespace {
    struct RGB { int r, g, b; };
    struct HSL { double h, s, l; };

    RGB ParseHex(const std::string& hex) {
        std::string clean = hex;
        if (!clean.empty() && clean[0] == '#') clean = clean.substr(1);

        if (clean.size() != 6) {
            throw std::runtime_error("Invalid HEX color: expected format #RRGGBB");
        }

        for (char c : clean) {
            if (!std::isxdigit((unsigned char)c)) {
                throw std::runtime_error("Invalid HEX color: contains non-hex characters");
            }
        }

        int r = std::stoi(clean.substr(0, 2), nullptr, 16);
        int g = std::stoi(clean.substr(2, 2), nullptr, 16);
        int b = std::stoi(clean.substr(4, 2), nullptr, 16);
        return { r, g, b };
    }

    // Конвертация RGB -> HSL
    HSL RgbToHsl(const RGB& rgb) {
        double r = rgb.r / 255.0, g = rgb.g / 255.0, b = rgb.b / 255.0;
        double maxVal = std::max({ r, g, b });
        double minVal = std::min({ r, g, b });
        double h = 0, s = 0, l = (maxVal + minVal) / 2.0;

        if (maxVal != minVal) {
            double d = maxVal - minVal;
            s = l > 0.5 ? d / (2.0 - maxVal - minVal) : d / (maxVal + minVal);

            if (maxVal == r) h = (g - b) / d + (g < b ? 6 : 0);
            else if (maxVal == g) h = (b - r) / d + 2;
            else h = (r - g) / d + 4;
            h /= 6.0;
        }

        return { h * 360.0, s * 100.0, l * 100.0 };
    }

    std::string RgbToHex(const RGB& rgb) {
        std::ostringstream ss;
        ss << "#" << std::hex << std::setfill('0')
            << std::setw(2) << rgb.r
            << std::setw(2) << rgb.g
            << std::setw(2) << rgb.b;
        return ss.str();
    }
}

std::string ColorTool::Execute(const std::string& action, const std::string& payload) {
    if (action != "convert") {
        throw std::runtime_error("Unknown action: " + action);
    }

    RGB rgb = ParseHex(payload);
    HSL hsl = RgbToHsl(rgb);

    std::ostringstream rgbStr, hslStr;
    rgbStr << "rgb(" << rgb.r << ", " << rgb.g << ", " << rgb.b << ")";
    hslStr << "hsl(" << (int)std::round(hsl.h) << ", "
        << (int)std::round(hsl.s) << "%, "
        << (int)std::round(hsl.l) << "%)";

    json result = {
        {"hex", RgbToHex(rgb)},
        {"rgb", rgbStr.str()},
        {"hsl", hslStr.str()}
    };

    return result.dump(4, ' ', false);
}