#pragma once
#include <windows.h>
#include <wrl.h>
#include "WebView2.h"

class WebViewHost
{
public:
    void Initialize(HWND parentWindow);
    void ResizeToWindow(HWND parentWindow);

private:
    Microsoft::WRL::ComPtr<ICoreWebView2Controller> m_controller;
    Microsoft::WRL::ComPtr<ICoreWebView2> m_webview;
};