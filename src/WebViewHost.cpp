#include "WebViewHost.h"
#include <shlwapi.h>
#include <string>
#pragma comment(lib, "shlwapi.lib")

using namespace Microsoft::WRL;

void WebViewHost::Initialize(HWND parentWindow)
{
    CreateCoreWebView2EnvironmentWithOptions(nullptr, nullptr, nullptr,
        Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
            [this, parentWindow](HRESULT result, ICoreWebView2Environment* env) -> HRESULT
            {
                env->CreateCoreWebView2Controller(parentWindow,
                    Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
                        [this, parentWindow](HRESULT result, ICoreWebView2Controller* controller) -> HRESULT
                        {
                            if (controller)
                            {
                                m_controller = controller;
                                m_controller->get_CoreWebView2(&m_webview);
                            }

                            RECT bounds;
                            GetClientRect(parentWindow, &bounds);
                            m_controller->put_Bounds(bounds);

                            wchar_t exePath[MAX_PATH];
                            GetModuleFileNameW(nullptr, exePath, MAX_PATH);
                            PathRemoveFileSpecW(exePath);
                            std::wstring indexPath = std::wstring(exePath) + L"\\ui\\index.html";
                            std::wstring url = L"file:///" + indexPath;

                            m_webview->Navigate(url.c_str());

                            return S_OK;
                        }).Get());
                return S_OK;
            }).Get());
}

void WebViewHost::ResizeToWindow(HWND parentWindow)
{
    if (m_controller)
    {
        RECT bounds;
        GetClientRect(parentWindow, &bounds);
        m_controller->put_Bounds(bounds);
    }
}