#include "pch.h"
#include "Window.h"

WindowKeyInputs Window::s_window_key_inputs = {};

LRESULT CALLBACK Window::HandleMsg(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CLOSE:
    {
        DestroyWindow(hwnd);
        return 0;
    }
    case WM_DESTROY:
    {
        PostQuitMessage(0);
        return 0;
    }
    case WM_KEYDOWN:
    {
        HandleInputs(wParam, true);;
    } break;

    case WM_KEYUP:
    {
        HandleInputs(wParam, false);
    } break;
    }

    return DefWindowProc(hwnd, message, wParam, lParam);
}

void Window::HandleInputs(WPARAM wParam, bool key_down)
{
    switch (wParam)
    {
    case 0x41:
        Window::s_window_key_inputs.a_key = key_down;
        break;
    case 0x44:
        Window::s_window_key_inputs.d_key = key_down;
        break;
    case 0x57:
        Window::s_window_key_inputs.w_key = key_down;
        break;
    case 0x53:
        Window::s_window_key_inputs.s_key = key_down;
        break;
    case VK_SHIFT:
        Window::s_window_key_inputs.shift_key = key_down;
        break;
    case VK_CONTROL:
        Window::s_window_key_inputs.left_control_key = key_down;
        break;
    case 0x51:
        Window::s_window_key_inputs.q_key = key_down;
        break;
    case 0x45:
        Window::s_window_key_inputs.e_key = key_down;
        break;
    }
}

Window::Window(UINT width, UINT height, std::wstring window_name, std::wstring window_text)
{
    m_window_height = height;
    m_window_width = width;

    WNDCLASSEX wc = { };

    wc.cbSize = sizeof(WNDCLASSEX);
    wc.hInstance = m_window_hinstance;
    wc.lpszClassName = window_name.c_str();
    //Set msg handler to the wndproc
    wc.lpfnWndProc = Window::HandleMsg;

    RegisterClassEx(&wc);

    RECT rt = { 0, 0, static_cast<LONG>(m_window_width),
        static_cast<LONG>(m_window_height) };
    AdjustWindowRect(&rt, WS_OVERLAPPEDWINDOW, FALSE);

    m_window_handle = CreateWindowEx(0, window_name.c_str(), window_text.c_str(),
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, rt.right - rt.left, rt.bottom - rt.top,
        nullptr, nullptr, m_window_hinstance, nullptr);

    if (m_window_handle == nullptr)
    {
        DWORD errorCode = GetLastError();
    }

    ShowWindow(m_window_handle, SW_SHOWNORMAL);
}

bool Window::WinMsg()
{
    MSG msg{};
    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        if (msg.message == WM_QUIT)
        {
            return false;
        }
    }
    return true;
}

HWND Window::GetWindowHandle() const
{
    return m_window_handle;
}

float Window::GetWindowHeight() const
{
    return (float)m_window_height;
}

float Window::GetWindowWidth() const
{
    return (float)m_window_width;
}
