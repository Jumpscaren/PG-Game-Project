#include "pch.h"
#include "Input.h"
#include "Keyboard.h"
#include "Scripting/CSMonoCore.h"

void InputInterface::RegisterInterface(CSMonoCore* mono_core)
{
    auto input_class = mono_core->RegisterMonoClass("ScriptProject.Engine", "Input");
	mono_core->HookAndRegisterMonoMethodType<InputInterface::GetKeyPressed>(input_class, "GetKeyPressed", InputInterface::GetKeyPressed);
    mono_core->HookAndRegisterMonoMethodType<InputInterface::GetKeyDown>(input_class, "GetKeyDown", InputInterface::GetKeyDown);
}

bool InputInterface::GetKeyPressed()
{
    return Keyboard::Get()->GetKeyPressed(Keyboard::Key::D);
}

bool InputInterface::GetKeyDown()
{
    return Keyboard::Get()->GetKeyDown(Keyboard::Key::D);
}
