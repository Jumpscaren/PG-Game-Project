#pragma once
#include "Scripting/CSMonoObject.h"
#include "SceneSystem/SceneDefines.h"
#include "ECS/EntityDefinition.h"

class EntityManager;

class ListSetInterface
{
public:
	static void RegisterInterface(CSMonoCore* mono_core);

public:
	static void AddGameObject(const CSMonoObject& list, const CSMonoObject& game_object);

private:
	static MonoMethodHandle add_list_set_game_object_method_handle;
};

