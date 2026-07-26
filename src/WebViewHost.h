#pragma once
#include <windows.h>
#include <wrl.h>
#include <string>
#include "WebView2.h"

#define WM_APP_WEBVIEW_PUSH (WM_APP + 1)

class WebViewHost
{
public:
    void Initialize(HWND parentWindow, const std::wstring& uiRootDir);
    void ResizeToWindow(HWND parentWindow);
    void PostMessageToWebView(const std::wstring& message);
    void PostMessageToWebViewThreadSafe(const std::wstring& message);
    void HandleThreadSafeMessage(LPARAM lParam);

private:
    void SetupMessageBridge();

    HWND m_hwnd = nullptr;
    std::wstring m_uiRootDir;
    Microsoft::WRL::ComPtr<ICoreWebView2Controller> m_controller;
    Microsoft::WRL::ComPtr<ICoreWebView2> m_webview;
};