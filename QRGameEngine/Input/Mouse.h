#pragma once
class Mouse
{
public:
	enum class MouseButton : uint8_t
	{
		LEFT = 0, RIGHT, WHEEL
		//Etc for more mouse buttons
	};


private:
	enum class MousePress : uint8_t
	{
		UP = 0,
		DOWN_PRESSED = 1,
		DOWN = 2
	};

private:
	struct MouseButtonPressed
	{
		MousePress left, right, wheel;
	} mouse_button_pressed;

	static Mouse* s_mouse;

private:
	void SetMouseButton(const MouseButton& mouse_button, bool pressed);
	void MouseButtonAlreadyPressed(MousePress& mouse_button_press, bool pressed, bool update_mouse_button = false);
	MousePress GetMouseButtonPress(const MouseButton& mouse_button);

public:
	Mouse();
	static Mouse* Get();

	void MouseButtonDown(const MouseButton& mouse_button);
	void MouseButtonUp(const MouseButton& mouse_button);

	bool GetMouseButtonPressed(const MouseButton& mouse_button);
	bool GetMouseButtonDown(const MouseButton& mouse_button);

	void UpdateMouseButtons(bool pressed = true);
};

