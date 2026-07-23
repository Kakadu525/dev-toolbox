#include <windows.h>
#include <memory>
#include "WebViewHost.h"
#include "ToolRegistry.h"
#include "tools/Base64Tool.h"
#include "tools/HashTool.h"
#include "tools/UuidTool.h"
#include "tools/JsonTool.h"

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

WebViewHost* g_webviewHost = nullptr;

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int nCmdShow)
{
    ToolRegistry::Instance().Register("base64", std::make_unique<Base64Tool>());
    ToolRegistry::Instance().Register("hash", std::make_unique<HashTool>());
    ToolRegistry::Instance().Register("uuid", std::make_unique<UuidTool>());
    ToolRegistry::Instance().Register("json", std::make_unique<JsonTool>());

    const wchar_t CLASS_NAME[] = L"DevToolboxWindowClass";

    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0, CLASS_NAME, L"Dev Toolbox",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 1200, 800,
        nullptr, nullptr, hInstance, nullptr
    );

    if (!hwnd) return 0;

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    g_webviewHost = new WebViewHost();
    g_webviewHost->Initialize(hwnd);

    MSG msg = {};
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    delete g_webviewHost;
    return 0;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_SIZE:
        if (g_webviewHost)
            g_webviewHost->ResizeToWindow(hwnd);
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}