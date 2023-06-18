#include "pch.h"
#include "Input.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "Scripting/CSMonoCore.h"

void InputInterface::RegisterInterface(CSMonoCore* mono_core)
{
    auto input_class = mono_core->RegisterMonoClass("ScriptProject.Engine", "Input");
	mono_core->HookAndRegisterMonoMethodType<InputInterface::GetKeyPressed>(input_class, "GetKeyPressed", InputInterface::GetKeyPressed);
    mono_core->HookAndRegisterMonoMethodType<InputInterface::GetKeyDown>(input_class, "GetKeyDown", InputInterface::GetKeyDown);
    mono_core->HookAndRegisterMonoMethodType<InputInterface::GetMouseButtonPressed>(input_class, "GetMouseButtonPressed", InputInterface::GetMouseButtonPressed);
    mono_core->HookAndRegisterMonoMethodType<InputInterface::GetMouseButtonDown>(input_class, "GetMouseButtonDown", InputInterface::GetMouseButtonDown);
    mono_core->HookAndRegisterMonoMethodType<InputInterface::GetMouseWheelSpin>(input_class, "GetMouseWheelSpin", InputInterface::GetMouseWheelSpin);
}

bool InputInterface::GetKeyPressed(int key)
{
    return Keyboard::Get()->GetKeyPressed(Keyboard::Key(key));
}

bool InputInterface::GetKeyDown(int key)
{
    return Keyboard::Get()->GetKeyDown(Keyboard::Key(key));
}

bool InputInterface::GetMouseButtonPressed(int mouse_button)
{
    return Mouse::Get()->GetMouseButtonPressed(Mouse::MouseButton(mouse_button));
}

bool InputInterface::GetMouseButtonDown(int mouse_button)
{
    return Mouse::Get()->GetMouseButtonDown(Mouse::MouseButton(mouse_button));
}

bool InputInterface::GetMouseWheelSpin(int direction)
{
    return Mouse::Get()->GetMouseWheelSpinDirection(Mouse::MouseWheelSpin(direction));
}
