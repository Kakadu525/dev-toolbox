#include "WebViewHost.h"
#include "StringUtil.h"
#include "ToolRegistry.h"
#include <shlwapi.h>
#include <string>
#pragma comment(lib, "shlwapi.lib")

using namespace Microsoft::WRL;

void WebViewHost::Initialize(HWND parentWindow)
{
    m_hwnd = parentWindow;

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

                            SetupMessageBridge();

                            ToolRegistry::Instance().SetPushCallback(
                                [this](const std::string& utf8Message) {
                                    std::wstring wideMessage = Utf8ToWide(utf8Message);
                                    PostMessageToWebViewThreadSafe(wideMessage);
                                });

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

void WebViewHost::SetupMessageBridge()
{
    EventRegistrationToken token;
    m_webview->add_WebMessageReceived(
        Callback<ICoreWebView2WebMessageReceivedEventHandler>(
            [this](ICoreWebView2* sender, ICoreWebView2WebMessageReceivedEventArgs* args) -> HRESULT
            {
                LPWSTR message = nullptr;
                args->TryGetWebMessageAsString(&message);
                if (message)
                {
                    std::wstring received(message);
                    CoTaskMemFree(message);

                    std::string requestUtf8 = WideToUtf8(received);
                    std::string responseUtf8 = ToolRegistry::Instance().Handle(requestUtf8);
                    std::wstring responseWide = Utf8ToWide(responseUtf8);

                    m_webview->PostWebMessageAsString(responseWide.c_str());
                }
                return S_OK;
            }).Get(), &token);
}

void WebViewHost::PostMessageToWebView(const std::wstring& message)
{
    if (m_webview) {
        m_webview->PostWebMessageAsString(message.c_str());
    }
}

void WebViewHost::PostMessageToWebViewThreadSafe(const std::wstring& message)
{

    auto* heapMessage = new std::wstring(message);
    ::PostMessageW(m_hwnd, WM_APP_WEBVIEW_PUSH, 0, reinterpret_cast<LPARAM>(heapMessage));
}

void WebViewHost::HandleThreadSafeMessage(LPARAM lParam)
{
    auto* heapMessage = reinterpret_cast<std::wstring*>(lParam);
    if (heapMessage) {
        PostMessageToWebView(*heapMessage);
        delete heapMessage;
    }
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