#include "ClipboardTool.h"
#include "../StringUtil.h"
#include <stb_image.h>
#include <stb_image_write.h>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <algorithm>
#include <vector>
#include <cstring>

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

    void WriteCallback(void* context, void* data, int size) {
        auto* out = reinterpret_cast<std::vector<unsigned char>*>(context);
        auto* bytes = reinterpret_cast<unsigned char*>(data);
        out->insert(out->end(), bytes, bytes + size);
    }

    std::vector<unsigned char> DibToPngBytes(HANDLE hDib, int& outWidth, int& outHeight) {
        BITMAPINFO* bmi = static_cast<BITMAPINFO*>(GlobalLock(hDib));
        if (!bmi) throw std::runtime_error("Failed to lock DIB data");

        int width = bmi->bmiHeader.biWidth;
        int height = std::abs(bmi->bmiHeader.biHeight);

        int paletteEntries = 0;
        if (bmi->bmiHeader.biBitCount <= 8) {
            paletteEntries = bmi->bmiHeader.biClrUsed ? bmi->bmiHeader.biClrUsed : (1 << bmi->bmiHeader.biBitCount);
        }
        size_t paletteSize = (size_t)paletteEntries * sizeof(RGBQUAD);
        unsigned char* srcPixels = reinterpret_cast<unsigned char*>(bmi) + bmi->bmiHeader.biSize + paletteSize;

        HDC hdcScreen = GetDC(nullptr);

        BITMAPINFO targetInfo = {};
        targetInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        targetInfo.bmiHeader.biWidth = width;
        targetInfo.bmiHeader.biHeight = -height; // top-down, для предсказуемого порядка строк
        targetInfo.bmiHeader.biPlanes = 1;
        targetInfo.bmiHeader.biBitCount = 32;
        targetInfo.bmiHeader.biCompression = BI_RGB;

        void* pBits = nullptr;
        HBITMAP hTargetBmp = CreateDIBSection(hdcScreen, &targetInfo, DIB_RGB_COLORS, &pBits, nullptr, 0);
        if (!hTargetBmp) {
            ReleaseDC(nullptr, hdcScreen);
            GlobalUnlock(hDib);
            throw std::runtime_error("Failed to create DIB section");
        }

        HDC hdcMem = CreateCompatibleDC(hdcScreen);
        HBITMAP hOldBmp = static_cast<HBITMAP>(SelectObject(hdcMem, hTargetBmp));

        StretchDIBits(hdcMem, 0, 0, width, height, 0, 0, width, height,
            srcPixels, bmi, DIB_RGB_COLORS, SRCCOPY);

        std::vector<unsigned char> rgba((size_t)width * height * 4);
        unsigned char* src = static_cast<unsigned char*>(pBits);
        for (int i = 0; i < width * height; i++) {
            rgba[i * 4 + 0] = src[i * 4 + 2]; // R <- B
            rgba[i * 4 + 1] = src[i * 4 + 1]; // G
            rgba[i * 4 + 2] = src[i * 4 + 0]; // B <- R
            rgba[i * 4 + 3] = 255;            // A 
        }

        SelectObject(hdcMem, hOldBmp);
        DeleteDC(hdcMem);
        DeleteObject(hTargetBmp);
        ReleaseDC(nullptr, hdcScreen);
        GlobalUnlock(hDib);

        outWidth = width;
        outHeight = height;

        std::vector<unsigned char> pngBytes;
        stbi_write_png_to_func(WriteCallback, &pngBytes, width, height, 4, rgba.data(), width * 4);
        return pngBytes;
    }

    HGLOBAL PngBase64ToDibGlobal(const std::string& base64Png) {
        std::vector<unsigned char> pngBytes = DecodeBase64(base64Png);

        int width, height, channels;
        unsigned char* pixels = stbi_load_from_memory(
            pngBytes.data(), (int)pngBytes.size(), &width, &height, &channels, 4);
        if (!pixels) throw std::runtime_error("Failed to decode image for clipboard restore");

        BITMAPINFOHEADER header = {};
        header.biSize = sizeof(BITMAPINFOHEADER);
        header.biWidth = width;
        header.biHeight = height;
        header.biPlanes = 1;
        header.biBitCount = 32;
        header.biCompression = BI_RGB;

        size_t pixelDataSize = (size_t)width * height * 4;
        size_t totalSize = sizeof(BITMAPINFOHEADER) + pixelDataSize;

        HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, totalSize);
        if (!hMem) {
            stbi_image_free(pixels);
            throw std::runtime_error("Failed to allocate clipboard memory");
        }

        unsigned char* pMem = static_cast<unsigned char*>(GlobalLock(hMem));
        memcpy(pMem, &header, sizeof(BITMAPINFOHEADER));

        unsigned char* dst = pMem + sizeof(BITMAPINFOHEADER);
        for (int y = 0; y < height; y++) {
            int srcRow = height - 1 - y; 
            for (int x = 0; x < width; x++) {
                unsigned char* s = pixels + (srcRow * width + x) * 4;
                unsigned char* d = dst + (y * width + x) * 4;
                d[0] = s[2]; // B
                d[1] = s[1]; // G
                d[2] = s[0]; // R
                d[3] = 0;    // padding byte
            }
        }

        GlobalUnlock(hMem);
        stbi_image_free(pixels);
        return hMem;
    }
}

void ClipboardTool::TrimType(const std::string& type, size_t limit) {
    size_t count = 0;
    for (auto& e : m_history) if (e.type == type) count++;

    while (count > limit) {
        for (auto it = m_history.rbegin(); it != m_history.rend(); ++it) {
            if (it->type == type) {
                m_history.erase(std::next(it).base());
                count--;
                break;
            }
        }
    }
}

void ClipboardTool::OnClipboardChanged(HWND hwnd) {
    if (!OpenClipboard(hwnd)) return;

    bool handledImage = false;

    if (IsClipboardFormatAvailable(CF_DIB)) {
        HANDLE hDib = GetClipboardData(CF_DIB);
        if (hDib) {
            try {
                int width = 0, height = 0;
                std::vector<unsigned char> pngBytes = DibToPngBytes(hDib, width, height);
                std::string base64Png = EncodeBase64(pngBytes);

                std::lock_guard<std::mutex> lock(m_mutex);
                if (m_history.empty() || m_history.front().type != "image" || m_history.front().content != base64Png) {
                    ClipboardEntry entry;
                    entry.type = "image";
                    entry.content = base64Png;
                    entry.width = width;
                    entry.height = height;
                    m_history.push_front(entry);
                    TrimType("image", kMaxImageEntries);
                }
                handledImage = true;
            }
            catch (...) {
            
            }
        }
    }

    if (!handledImage && IsClipboardFormatAvailable(CF_UNICODETEXT)) {
        HANDLE hData = GetClipboardData(CF_UNICODETEXT);
        if (hData) {
            wchar_t* pText = static_cast<wchar_t*>(GlobalLock(hData));
            if (pText) {
                std::wstring wideText(pText);
                GlobalUnlock(hData);
                std::string text = WideToUtf8(wideText);

                if (!text.empty()) {
                    std::lock_guard<std::mutex> lock(m_mutex);
                    if (m_history.empty() || m_history.front().type != "text" || m_history.front().content != text) {
                        ClipboardEntry entry;
                        entry.type = "text";
                        entry.content = text;
                        m_history.push_front(entry);
                        TrimType("text", kMaxTextEntries);
                    }
                }
            }
        }
    }

    CloseClipboard();
}

std::string ClipboardTool::Execute(const std::string& action, const std::string& payload) {
    if (action == "list") {
        std::lock_guard<std::mutex> lock(m_mutex);
        json history = json::array();
        for (const auto& entry : m_history) {
            history.push_back({
                {"type", entry.type},
                {"content", entry.content},
                {"width", entry.width},
                {"height", entry.height}
                });
        }
        json result = { {"history", history} };
        return result.dump();
    }

    if (action == "restore") {
        int index;
        try {
            index = std::stoi(payload);
        }
        catch (...) {
            throw std::runtime_error("Invalid index: " + payload);
        }

        ClipboardEntry entry;
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            if (index < 0 || index >= (int)m_history.size()) {
                throw std::runtime_error("Index out of range");
            }
            entry = m_history[index];
        }

        if (!OpenClipboard(nullptr)) {
            throw std::runtime_error("Failed to open clipboard");
        }
        EmptyClipboard();

        if (entry.type == "image") {
            HGLOBAL hDib = PngBase64ToDibGlobal(entry.content);
            if (!SetClipboardData(CF_DIB, hDib)) {
                GlobalFree(hDib);
            }
        }
        else {
            std::wstring wideText = Utf8ToWide(entry.content);
            size_t bytes = (wideText.size() + 1) * sizeof(wchar_t);
            HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, bytes);
            if (hMem) {
                void* pMem = GlobalLock(hMem);
                memcpy(pMem, wideText.c_str(), bytes);
                GlobalUnlock(hMem);
                if (!SetClipboardData(CF_UNICODETEXT, hMem)) {
                    GlobalFree(hMem);
                }
            }
        }

        CloseClipboard();

        json result = { {"restored", true} };
        return result.dump();
    }

    if (action == "clear") {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_history.clear();
        json result = { {"cleared", true} };
        return result.dump();
    }

    throw std::runtime_error("Unknown action: " + action);
}