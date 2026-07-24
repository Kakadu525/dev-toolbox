#include "ImageTool.h"
#include <stb_image.h>
#include <stb_image_write.h>
#include <stb_image_resize2.h>
#include <nlohmann/json.hpp>
#include <vector>
#include <stdexcept>

using json = nlohmann::json;

namespace {
    const std::string kChars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";

    std::string EncodeBase64(const std::vector<unsigned char>& data) {
        std::string result;
        int val = 0, valb = -6;
        for (unsigned char c : data) {
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

    std::vector<unsigned char> DecodeBase64(const std::string& input) {
        std::vector<int> table(256, -1);
        for (int i = 0; i < 64; i++) table[(unsigned char)kChars[i]] = i;

        std::vector<unsigned char> result;
        int val = 0, valb = -8;
        for (unsigned char c : input) {
            if (c == '=') break;
            if (table[c] == -1) continue;
            val = (val << 6) + table[c];
            valb += 6;
            if (valb >= 0) {
                result.push_back((unsigned char)((val >> valb) & 0xFF));
                valb -= 8;
            }
        }
        return result;
    }

    // stb_image_write пишет результат через callback
    void WriteCallback(void* context, void* data, int size) {
        auto* out = reinterpret_cast<std::vector<unsigned char>*>(context);
        auto* bytes = reinterpret_cast<unsigned char*>(data);
        out->insert(out->end(), bytes, bytes + size);
    }
}

std::string ImageTool::Execute(const std::string& action, const std::string& payload) {
    if (action != "convert") {
        throw std::runtime_error("Unknown action: " + action);
    }

    json request;
    try {
        request = json::parse(payload);
    }
    catch (const json::parse_error&) {
        throw std::runtime_error("Invalid request format");
    }

    std::string imageBase64 = request.value("image", "");
    std::string format = request.value("format", "png");
    int quality = request.value("quality", 90);
    int targetWidth = request.value("width", 0);
    int targetHeight = request.value("height", 0);

    if (imageBase64.empty()) {
        throw std::runtime_error("Image data is required");
    }

    std::vector<unsigned char> inputBytes = DecodeBase64(imageBase64);

    // Загружаем картинку, принудительно приводим к RGBA
    int width, height, channels;
    unsigned char* pixels = stbi_load_from_memory(
        inputBytes.data(), (int)inputBytes.size(), &width, &height, &channels, 4);

    if (!pixels) {
        throw std::runtime_error(std::string("Failed to decode image: ") + stbi_failure_reason());
    }

    unsigned char* finalPixels = pixels;
    int finalWidth = width, finalHeight = height;
    std::vector<unsigned char> resizedBuffer;

    bool needsResize = (targetWidth > 0 && targetHeight > 0) &&
        (targetWidth != width || targetHeight != height);

    if (needsResize) {
        resizedBuffer.resize((size_t)targetWidth * targetHeight * 4);
        unsigned char* resizeResult = stbir_resize_uint8_linear(
            pixels, width, height, 0,
            resizedBuffer.data(), targetWidth, targetHeight, 0,
            STBIR_RGBA);

        if (!resizeResult) {
            stbi_image_free(pixels);
            throw std::runtime_error("Failed to resize image");
        }

        finalPixels = resizedBuffer.data();
        finalWidth = targetWidth;
        finalHeight = targetHeight;
    }

    std::vector<unsigned char> outputBytes;
    int writeOk = 0;

    if (format == "png") {
        writeOk = stbi_write_png_to_func(WriteCallback, &outputBytes,
            finalWidth, finalHeight, 4, finalPixels, finalWidth * 4);
    }
    else if (format == "jpeg" || format == "jpg") {
        writeOk = stbi_write_jpg_to_func(WriteCallback, &outputBytes,
            finalWidth, finalHeight, 4, finalPixels, quality);
    }
    else if (format == "bmp") {
        writeOk = stbi_write_bmp_to_func(WriteCallback, &outputBytes,
            finalWidth, finalHeight, 4, finalPixels);
    }
    else {
        stbi_image_free(pixels);
        throw std::runtime_error("Unsupported format: " + format);
    }

    stbi_image_free(pixels);

    if (!writeOk) {
        throw std::runtime_error("Failed to encode output image");
    }

    json result = {
        {"image", EncodeBase64(outputBytes)},
        {"format", format},
        {"width", finalWidth},
        {"height", finalHeight},
        {"sizeBytes", (int)outputBytes.size()}
    };

    return result.dump();
}