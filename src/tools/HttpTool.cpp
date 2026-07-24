#include "HttpTool.h"
#include "../StringUtil.h"
#include <windows.h>
#include <winhttp.h>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <chrono>
#include <vector>
#pragma comment(lib, "winhttp.lib")

using json = nlohmann::json;

namespace {
    struct WinHttpHandle {
        HINTERNET h = nullptr;
        ~WinHttpHandle() { if (h) WinHttpCloseHandle(h); }
        operator HINTERNET() const { return h; }
    };

    std::string GetLastErrorMessage() {
        DWORD err = GetLastError();
        LPWSTR buf = nullptr;
        FormatMessageW(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_FROM_HMODULE,
            GetModuleHandleW(L"winhttp.dll"), err, 0, (LPWSTR)&buf, 0, nullptr);
        std::string result = buf ? WideToUtf8(buf) : ("WinHTTP error code " + std::to_string(err));
        if (buf) LocalFree(buf);
        return result;
    }
}

std::string HttpTool::Execute(const std::string& action, const std::string& payload) {
    if (action != "send") {
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

    std::wstring wUrl = Utf8ToWide(url);

    // Разбираем URL на составляющие 
    URL_COMPONENTS urlComp = {};
    urlComp.dwStructSize = sizeof(urlComp);
    wchar_t hostBuf[256] = {};
    wchar_t pathBuf[2048] = {};
    urlComp.lpszHostName = hostBuf;
    urlComp.dwHostNameLength = 256;
    urlComp.lpszUrlPath = pathBuf;
    urlComp.dwUrlPathLength = 2048;

    if (!WinHttpCrackUrl(wUrl.c_str(), (DWORD)wUrl.size(), 0, &urlComp)) {
        throw std::runtime_error("Invalid URL");
    }

    bool isHttps = (urlComp.nScheme == INTERNET_SCHEME_HTTPS);

    WinHttpHandle hSession;
    hSession.h = WinHttpOpen(L"DevToolbox/1.0",
        WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
        WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (!hSession.h) throw std::runtime_error("Failed to init WinHTTP session: " + GetLastErrorMessage());

    // Таймауты, чтобы зависший сервер не морозил приложение 
    WinHttpSetTimeouts(hSession, 10000, 10000, 10000, 15000);

    WinHttpHandle hConnect;
    hConnect.h = WinHttpConnect(hSession, urlComp.lpszHostName, urlComp.nPort, 0);
    if (!hConnect.h) throw std::runtime_error("Failed to connect: " + GetLastErrorMessage());

    std::wstring wPath = urlComp.lpszUrlPath[0] ? urlComp.lpszUrlPath : L"/";
    std::wstring wMethod = Utf8ToWide(method);

    WinHttpHandle hRequest;
    hRequest.h = WinHttpOpenRequest(hConnect, wMethod.c_str(), wPath.c_str(),
        nullptr, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES,
        isHttps ? WINHTTP_FLAG_SECURE : 0);
    if (!hRequest.h) throw std::runtime_error("Failed to open request: " + GetLastErrorMessage());


    for (const auto& header : headers) {
        std::string key = header.value("key", "");
        std::string value = header.value("value", "");
        if (key.empty()) continue;
        std::wstring headerLine = Utf8ToWide(key + ": " + value);
        WinHttpAddRequestHeaders(hRequest, headerLine.c_str(), (DWORD)-1L, WINHTTP_ADDREQ_FLAG_ADD);
    }

    auto startTime = std::chrono::steady_clock::now();

    BOOL sendOk = WinHttpSendRequest(hRequest,
        WINHTTP_NO_ADDITIONAL_HEADERS, 0,
        body.empty() ? WINHTTP_NO_REQUEST_DATA : (LPVOID)body.data(),
        (DWORD)body.size(), (DWORD)body.size(), 0);

    if (!sendOk) throw std::runtime_error("Failed to send request: " + GetLastErrorMessage());

    if (!WinHttpReceiveResponse(hRequest, nullptr)) {
        throw std::runtime_error("Failed to receive response: " + GetLastErrorMessage());
    }

    auto endTime = std::chrono::steady_clock::now();
    long long elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();

    // Статус-код
    DWORD statusCode = 0;
    DWORD statusSize = sizeof(statusCode);
    WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
        WINHTTP_HEADER_NAME_BY_INDEX, &statusCode, &statusSize, WINHTTP_NO_HEADER_INDEX);

    DWORD headersSize = 0;
    WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_RAW_HEADERS_CRLF,
        WINHTTP_HEADER_NAME_BY_INDEX, WINHTTP_NO_OUTPUT_BUFFER, &headersSize, WINHTTP_NO_HEADER_INDEX);
    std::wstring rawHeaders;
    if (headersSize > 0) {
        rawHeaders.resize(headersSize / sizeof(wchar_t));
        WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_RAW_HEADERS_CRLF,
            WINHTTP_HEADER_NAME_BY_INDEX, rawHeaders.data(), &headersSize, WINHTTP_NO_HEADER_INDEX);
    }

    std::string responseBody;
    DWORD available = 0;
    do {
        if (!WinHttpQueryDataAvailable(hRequest, &available)) break;
        if (available == 0) break;

        std::vector<char> buffer(available);
        DWORD read = 0;
        if (!WinHttpReadData(hRequest, buffer.data(), available, &read)) break;
        responseBody.append(buffer.data(), read);
    } while (available > 0);

    json result = {
        {"status", (int)statusCode},
        {"headers", WideToUtf8(rawHeaders)},
        {"body", responseBody},
        {"timeMs", elapsedMs}
    };

    return result.dump();
}