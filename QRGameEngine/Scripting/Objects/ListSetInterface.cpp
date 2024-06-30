#include "pch.h"
#include "ListSetInterface.h"
#include "../CSMonoCore.h"

MonoMethodHandle ListSetInterface::add_list_set_game_object_method_handle;

void ListSetInterface::RegisterInterface(CSMonoCore* mono_core)
{
	const auto list_class_handle = CSMonoCore::Get()->RegisterMonoClass("ScriptProject.Engine", "ListSetGameObject");
	add_list_set_game_object_method_handle = CSMonoCore::Get()->RegisterMonoMethod(list_class_handle, "Add");
}

void ListSetInterface::AddGameObject(const CSMonoObject& list, const CSMonoObject& game_object)
{
	CSMonoCore::Get()->CallMethod(add_list_set_game_object_method_handle, list, game_object);
}
