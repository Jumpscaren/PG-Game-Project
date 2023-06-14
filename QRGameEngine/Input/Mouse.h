#pragma once
#include "Common/EngineTypes.h"

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

	struct MouseButtonPressed
	{
		MousePress left, right, wheel;
	} mouse_button_pressed;

private:
	static Mouse* s_mouse;

	Vector2u m_current_mouse_coords;
	Vector2i m_delta_mouse_coords[2];
	uint8_t m_delta_mouse_coords_index, m_delta_mouse_coords_old_index;

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

	void MouseMove(Vector2u mouse_coords);
	void MouseDeltaMove(Vector2i mouse_delta_coords);

	Vector2u GetMouseCoords() const;
	Vector2i GetDeltaMouseCoords() const;

	void ResetMouseDeltaCoords();
	void SwitchMouseDeltaCoords();


};

