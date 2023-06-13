#include "pch.h"
#include "Mouse.h"

Mouse* Mouse::s_mouse = nullptr;

void Mouse::SetMouseButton(const MouseButton& mouse_button, bool pressed)
{
	switch (mouse_button)
	{
	case MouseButton::LEFT:
		MouseButtonAlreadyPressed(mouse_button_pressed.left, pressed);
		break;
	case MouseButton::RIGHT:
		MouseButtonAlreadyPressed(mouse_button_pressed.right, pressed);
		break;
	case MouseButton::WHEEL:
		MouseButtonAlreadyPressed(mouse_button_pressed.wheel, pressed);
		break;
	}
}

void Mouse::MouseButtonAlreadyPressed(MousePress& mouse_button_press, bool pressed, bool update_mouse_button)
{
	if (!pressed)
	{
		mouse_button_press = MousePress::UP;
		return;
	}
	if (pressed && update_mouse_button && mouse_button_press == MousePress::UP)
	{
		return;
	}
	if (pressed && mouse_button_press == MousePress::DOWN)
	{
		return;
	}
	if (pressed && mouse_button_press == MousePress::DOWN_PRESSED)
	{
		mouse_button_press = MousePress::DOWN;
		return;
	}
	mouse_button_press = MousePress::DOWN_PRESSED;
}

Mouse::MousePress Mouse::GetMouseButtonPress(const MouseButton& mouse_button)
{
	switch (mouse_button)
	{
	case MouseButton::LEFT:
		return mouse_button_pressed.left;
	case MouseButton::RIGHT:
		return mouse_button_pressed.right;
	case MouseButton::WHEEL:
		return mouse_button_pressed.wheel;
	}
}

Mouse::Mouse()
{
	s_mouse = this;
	UpdateMouseButtons(false);
}

Mouse* Mouse::Get()
{
	return s_mouse;
}

void Mouse::MouseButtonDown(const MouseButton& mouse_button)
{
	SetMouseButton(mouse_button, true);
}

void Mouse::MouseButtonUp(const MouseButton& mouse_button)
{
	SetMouseButton(mouse_button, false);
}

bool Mouse::GetMouseButtonPressed(const MouseButton& mouse_button)
{
	return GetMouseButtonPress(mouse_button) == MousePress::DOWN_PRESSED;
}

bool Mouse::GetMouseButtonDown(const MouseButton& mouse_button)
{
	MousePress mouse_button_press = GetMouseButtonPress(mouse_button);
	return (mouse_button_press == MousePress::DOWN) || (mouse_button_press == MousePress::DOWN_PRESSED);
}

void Mouse::UpdateMouseButtons(bool pressed)
{
	MouseButtonAlreadyPressed(mouse_button_pressed.left, pressed, true);
	MouseButtonAlreadyPressed(mouse_button_pressed.right, pressed, true);
	MouseButtonAlreadyPressed(mouse_button_pressed.wheel, pressed, true);
}
