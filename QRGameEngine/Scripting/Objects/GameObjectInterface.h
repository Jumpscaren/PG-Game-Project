#pragma once
#include "Scripting/CSMonoObject.h"
#include "ECS/EntityManager.h"

class GameObjectInterface
{
private:
	static MonoMethodHandle get_scene_index_method;
	static MonoFieldHandle get_entity_id_field;
	static MonoClassHandle game_object_class;

public:
	static void RegisterInterface(CSMonoCore* mono_core);

public:
	static Entity GetEntityID(const CSMonoObject& game_object);
	static SceneIndex GetSceneIndex(const CSMonoObject& game_object);
};

