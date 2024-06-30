#pragma once

struct WindowKeyInputs
{
	bool w_key = false;
	bool a_key = false;
	bool s_key = false;
	bool d_key = false;
	bool shift_key = false;
	bool left_control_key = false;

	bool q_key = false;
	bool e_key = false;

};

class Window
{
private:
	UINT m_window_height, m_window_width;
	HWND m_window_handle;
	HINSTANCE m_window_hinstance;

public:
	static WindowKeyInputs s_window_key_inputs;

private:
	static LRESULT CALLBACK HandleMsg(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static void HandleInputs(WPARAM wParam, bool key_down);

public:
	Window() = delete;
	Window(UINT width, UINT height, std::wstring window_name, std::wstring window_text);
	bool WinMsg();
	HWND GetWindowHandle() const;
	float GetWindowHeight() const;
	float GetWindowWidth() const;
	void SetWindowHeight(UINT height);
	void SetWindowWidth(UINT width);
};

