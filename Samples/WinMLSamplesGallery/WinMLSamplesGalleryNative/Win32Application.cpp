#include "pch.h"
#include "stdafx.h"
#include "Win32Application.h"

HWND Win32Application::m_hwnd = nullptr;
bool Win32Application::m_closeWindow = false;
std::wstring Win32Application::m_appPath = L"";

// Get the HModule that will be used to spawn
// the HWND
HMODULE GetCurrentModule()
{
    HMODULE hModule = NULL;
    GetModuleHandleEx(
        GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
        (LPCTSTR)GetCurrentModule,
        &hModule);
    return hModule;
}

// Launch the HWND and init D3D
int Win32Application::Run(D3D12Quad* pSample, int nCmdShow)
{
    // Get the Hinstance
    HINSTANCE hInstance = GetCurrentModule();

    // Initialize the window class.
    WNDCLASSEX windowClass = { 0 };
    windowClass.cbSize = sizeof(WNDCLASSEX);
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = WindowProc;
    windowClass.hInstance = hInstance;
    windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    windowClass.lpszClassName = L"DXQuad";

    auto hr = RegisterClassEx(&windowClass);
    // Register window class
    auto err = GetLastError();
    if (!hr)
    {
        MessageBox(NULL, L"Error registering class",
            L"Error", MB_OK | MB_ICONERROR);
        return false;
    }

    RECT windowRect = { 0, 0, static_cast<LONG>(pSample->GetWidth()), static_cast<LONG>(pSample->GetHeight()) };
    AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

    // Create the window and store a handle to it.
    m_hwnd = CreateWindow(
        windowClass.lpszClassName,
        pSample->GetTitle(),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        windowRect.right - windowRect.left,
        windowRect.bottom - windowRect.top,
        nullptr,        // We have no parent window.
        nullptr,        // We aren't using menus.
        hInstance,
        pSample);
    if (!m_hwnd) { MessageBox(NULL, L"Error creating window", L"Error", MB_OK | MB_ICONERROR); return false; }

    // Initialize the sample and show the window
    pSample->OnInit();
    ShowWindow(m_hwnd, nCmdShow);

    // Main sample loop.
    m_closeWindow = false;
    MSG msg = {};
    while (GetMessage(&msg, m_hwnd, 0, 0) != 0)
    {
        if (m_closeWindow)
        {
            pSample->OnDestroy();
            PostMessage(m_hwnd, WM_CLOSE, 0, 0);
            break;
        }
        else
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    // Unregister the hInstance so it can be reused if
    // the sample is reloaded
    if (!UnregisterClass(L"DXQuad", hInstance))
    {
        auto error = GetLastError();
        MessageBox(NULL, L"Error unregistering class",
            L"Error", MB_OK | MB_ICONERROR);
        return 1;
    }
    else {
        OutputDebugString(L"Class unregistered");
    }

    // Return this part of the WM_QUIT message to Windows.
    return static_cast<char>(msg.wParam);
}

// Main message handler for the sample.
LRESULT CALLBACK Win32Application::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    D3D12Quad* pSample = reinterpret_cast<D3D12Quad*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

    switch (message)
    {
    case WM_NCCREATE:
    {
        // Save the D3D12Quad* passed in to CreateWindow.
        LPCREATESTRUCT pCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
        SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pCreateStruct->lpCreateParams));
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;

    case WM_PAINT:
        if (pSample)
        {
            if (!m_closeWindow)
            {
                pSample->OnUpdate();
                pSample->OnRender();
            }
        }
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    // Handle any messages the switch statement didn't.
    return DefWindowProc(hWnd, message, wParam, lParam);
}

void Win32Application::CloseWindow()
{
    m_closeWindow = true;
}

void Win32Application::SetAppPath(winrt::hstring path)
{
    m_appPath = path;
}

std::wstring Win32Application::GetAssetPath(LPCWSTR assetName)
{
    return m_appPath + L"\\" + assetName;
}