#include "pch.h"
#include "Input.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "Scripting/CSMonoCore.h"
#include "Scripting/Objects/Vector2Interface.h"
#include "Components/CameraComponent.h"
#include "Scripting/Objects/GameObjectInterface.h"
#include "SceneSystem/SceneManager.h"

void InputInterface::RegisterInterface(CSMonoCore* mono_core)
{
    auto input_class = mono_core->RegisterMonoClass("ScriptProject.Engine", "Input");
	mono_core->HookAndRegisterMonoMethodType<InputInterface::GetKeyPressed>(input_class, "GetKeyPressed", InputInterface::GetKeyPressed);
    mono_core->HookAndRegisterMonoMethodType<InputInterface::GetKeyDown>(input_class, "GetKeyDown", InputInterface::GetKeyDown);
    mono_core->HookAndRegisterMonoMethodType<InputInterface::GetMouseButtonPressed>(input_class, "GetMouseButtonPressed", InputInterface::GetMouseButtonPressed);
    mono_core->HookAndRegisterMonoMethodType<InputInterface::GetMouseButtonDown>(input_class, "GetMouseButtonDown", InputInterface::GetMouseButtonDown);
    mono_core->HookAndRegisterMonoMethodType<InputInterface::GetMouseWheelSpin>(input_class, "GetMouseWheelSpin", InputInterface::GetMouseWheelSpin);
    mono_core->HookAndRegisterMonoMethodType<InputInterface::GetMousePosition>(input_class, "GetMousePosition", InputInterface::GetMousePosition);
    mono_core->HookAndRegisterMonoMethodType<InputInterface::GetMousePositionInWorld>(input_class, "GetMousePositionInWorld", InputInterface::GetMousePositionInWorld);
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

CSMonoObject InputInterface::GetMousePosition()
{
    const auto mouse_position = Mouse::Get()->GetMouseCoords();
    return Vector2Interface::CreateVector2(Vector2((float)mouse_position.x, (float)mouse_position.y));
}

CSMonoObject InputInterface::GetMousePositionInWorld(const CSMonoObject camera_game_object)
{
    const auto mouse_position = Mouse::Get()->GetMouseCoords();
    const auto camera_entity = GameObjectInterface::GetEntityID(camera_game_object);
    const auto camera_scene_index = GameObjectInterface::GetSceneIndex(camera_game_object);

    const auto mouse_world_position = CameraComponentInterface::ScreenToWorld(SceneManager::GetEntityManager(camera_scene_index)->GetComponent<CameraComponent>(camera_entity), Vector2((float)mouse_position.x, (float)mouse_position.y));
    return Vector2Interface::CreateVector2(Vector2(mouse_world_position.x, mouse_world_position.y));
}
