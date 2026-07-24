#include "QrTool.h"
#include "qrcodegen.hpp"
#include <nlohmann/json.hpp>
#include <sstream>
#include <stdexcept>

using json = nlohmann::json;
using qrcodegen::QrCode;

// Рисуем QR-код как SVG-path
namespace {
    std::string ToSvg(const QrCode& qr, int border) {
        std::ostringstream sb;
        int size = qr.getSize();
        int dim = size + border * 2;
        sb << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\" "
            << "viewBox=\"0 0 " << dim << " " << dim << "\" stroke=\"none\">\n";
        sb << "<rect width=\"100%\" height=\"100%\" fill=\"#FFFFFF\"/>\n";
        sb << "<path d=\"";
        for (int y = 0; y < size; y++) {
            for (int x = 0; x < size; x++) {
                if (qr.getModule(x, y)) {
                    if (x != 0 || y != 0) sb << " ";
                    sb << "M" << (x + border) << "," << (y + border) << "h1v1h-1z";
                }
            }
        }
        sb << "\" fill=\"#000000\"/>\n</svg>\n";
        return sb.str();
    }

    QrCode::Ecc ParseEcc(const std::string& level) {
        if (level == "LOW") return QrCode::Ecc::LOW;
        if (level == "QUARTILE") return QrCode::Ecc::QUARTILE;
        if (level == "HIGH") return QrCode::Ecc::HIGH;
        return QrCode::Ecc::MEDIUM; 
    }
}

std::string QrTool::Execute(const std::string& action, const std::string& payload) {
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

    std::string text = request.value("text", "");
    std::string eccLevel = request.value("ecc", "MEDIUM");

    if (text.empty()) {
        throw std::runtime_error("Text is required");
    }

    QrCode qr = [&]() {
        try {
            return QrCode::encodeText(text.c_str(), ParseEcc(eccLevel));
        }
        catch (const std::exception& e) {
            throw std::runtime_error(std::string("Cannot encode text into QR code: ") + e.what());
        }
        }();

    std::string svg = ToSvg(qr, 4);

    json result = { {"svg", svg} };
    return result.dump();
}