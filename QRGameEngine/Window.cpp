#include "pch.h"
#include "Window.h"
#include "Vendor/Include/ImGUI/imgui.h"
#include "Vendor/Include/ImGUI/backends/imgui_impl_win32.h"
#include "Vendor/Include/ImGUI/backends/imgui_impl_dx12.h"
#include "Input/Keyboard.h"
#include "Input/Mouse.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

WindowKeyInputs Window::s_window_key_inputs = {};

LRESULT CALLBACK Window::HandleMsg(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hwnd, message, wParam, lParam))
        return true;

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
    case WM_WINDOWPOSCHANGING:
    {
        //if (IsMinimized(Window::GetHandle())) return 0;
        //PublishEvent<WindowPosChangingEvent>();
        break;
    }
    case WM_SIZE:
    {
        //if (IsMinimized(Window::GetHandle())) return 0;

        //s_windowData.dimensions.x = LOWORD(lParam);
        //s_windowData.dimensions.y = HIWORD(lParam);

        //PublishEvent<WindowResizedEvent>(LOWORD(lParam), HIWORD(lParam));
        return 0;
    }
	case WM_KEYDOWN:
	{
		HandleInputs(wParam, true);
		Keyboard::Get()->KeyDown((Keyboard::Key)(uint8_t)(wParam));
	} break;

	case WM_KEYUP:
	{
		HandleInputs(wParam, false);
		Keyboard::Get()->KeyUp((Keyboard::Key)(uint8_t)(wParam));
	} break;
	case WM_LBUTTONDOWN:
	{
		Mouse::Get()->MouseButtonDown(Mouse::MouseButton::LEFT);
		//Mouse::OnButtonPressed(Button::Left);
		return 0;
	}
	case WM_LBUTTONUP:
	{
		Mouse::Get()->MouseButtonUp(Mouse::MouseButton::LEFT);
		//Mouse::OnButtonReleased(Button::Left);
		return 0;
	}
	case WM_RBUTTONDOWN:
	{
		Mouse::Get()->MouseButtonDown(Mouse::MouseButton::RIGHT);
		//Mouse::OnButtonPressed(Button::Right);
		return 0;
	}
	case WM_RBUTTONUP:
	{
		Mouse::Get()->MouseButtonUp(Mouse::MouseButton::RIGHT);
		//Mouse::OnButtonReleased(Button::Right);
		return 0;
	}
	case WM_MBUTTONDOWN:
	{
		Mouse::Get()->MouseButtonDown(Mouse::MouseButton::WHEEL);
		//Mouse::OnButtonPressed(Button::Wheel);
		return 0;
	}
	case WM_MBUTTONUP:
	{
		Mouse::Get()->MouseButtonUp(Mouse::MouseButton::WHEEL);
		//Mouse::OnButtonReleased(Button::Wheel);
		return 0;
	}
	//case WM_MOUSEACTIVATE:
	//{
	//	if (LOWORD(lParam) == HTTOP || LOWORD(lParam) == HTBOTTOM || LOWORD(lParam) == HTLEFT || LOWORD(lParam) == HTRIGHT ||
	//		LOWORD(lParam) == HTCLOSE || LOWORD(lParam) == HTMAXBUTTON || LOWORD(lParam) == HTMINBUTTON)
	//	{
	//		PublishEvent<WindowHitBorderEvent>();
	//		return MA_NOACTIVATE;
	//	}
	//	return MA_ACTIVATEANDEAT;
	//}
	//case WM_ACTIVATEAPP:
	//{
	//	PublishEvent<WindowActiveEvent>(static_cast<bool>(wParam));
	//	return 0;
	//}
	//case WM_SYSKEYDOWN:
	//{
	//	bool keyIsRepeated = (lParam >> 30) & 1;
	//	if (wParam == VK_RETURN && (lParam & (1 << 29) && !keyIsRepeated))
	//	{
	//		PublishEvent<WindowAltEnterEvent>(); // Handle ALT+ENTER:
	//	}
	//	if (wParam == VK_F4 && (lParam & (1 << 29) && !keyIsRepeated))
	//	{
	//		PublishEvent<WindowClosedEvent>();  // Handle ALT+F4
	//	}
	//}
	//case WM_KEYDOWN:
	//{
	//	bool keyIsRepeated = (lParam >> 30) & 1;
	//	if (!keyIsRepeated)
	//	{
	//		MapLeftRightShiftAndControl(wParam, lParam);
	//		Keyboard::OnKeyDown((Key)(u8)(wParam));
	//	}
	//	return 0;
	//}
	//case WM_SYSKEYUP:
	//case WM_KEYUP:
	//{
	//	MapLeftRightShiftAndControl(wParam, lParam);
	//	Keyboard::OnKeyUp((Key)(u8)(wParam));
	//	return 0;
	//}
	//case WM_LBUTTONDOWN:
	//{
	//	Mouse::OnButtonPressed(Button::Left);
	//	return 0;
	//}
	//case WM_LBUTTONUP:
	//{
	//	Mouse::OnButtonReleased(Button::Left);
	//	return 0;
	//}
	//case WM_RBUTTONDOWN:
	//{
	//	Mouse::OnButtonPressed(Button::Right);
	//	return 0;
	//}
	//case WM_RBUTTONUP:
	//{
	//	Mouse::OnButtonReleased(Button::Right);
	//	return 0;
	//}
	//case WM_MBUTTONDOWN:
	//{
	//	Mouse::OnButtonPressed(Button::Wheel);
	//	return 0;
	//}
	//case WM_MBUTTONUP:
	//{
	//	Mouse::OnButtonReleased(Button::Wheel);
	//	return 0;
	//}
	//case WM_MOUSEMOVE:
	//{
	//	// LOWORD and HIWORD can't be used because x and y can be negative
	//	POINTS point = MAKEPOINTS(lParam);
	//	if (point.x < 0 || point.y < 0 || point.x >= static_cast<int>(GetWidth()) || point.y >= static_cast<int>(GetHeight()))
	//	{
	//		std::cout << "Warning WM_MOUSEMOVE returned point outside the windows dimensions! x: " << point.x << " y: " << point.y << std::endl;
	//	}
	//	Mouse::OnMove({ static_cast<u32>(point.x), static_cast<u32>(point.y) });
	//	return 0;
	//}
	//case WM_INPUT:
	//{
	//	UINT size = 0u;
	//	if (GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT, nullptr, &size, sizeof(RAWINPUTHEADER)) == -1)
	//		return 0;

	//	std::vector<char> rawBuffer;
	//	rawBuffer.resize(size);

	//	if (GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT, rawBuffer.data(), &size, sizeof(RAWINPUTHEADER)) != size)
	//		return 0;

	//	auto& ri = reinterpret_cast<const RAWINPUT&>(*rawBuffer.data());
	//	if (ri.header.dwType == RIM_TYPEMOUSE && (ri.data.mouse.lLastX != 0 || ri.data.mouse.lLastY != 0))
	//	{
	//		Mouse::OnRawDelta({ ri.data.mouse.lLastX, ri.data.mouse.lLastY });
	//	}

	//	return 0;
	//}
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
