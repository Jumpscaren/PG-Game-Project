#pragma once
#include "Scripting/CSMonoObject.h"
#include "SceneSystem/SceneDefines.h"
#include "ECS/EntityDefinition.h"

class GameObjectInterface
{
private:
	static MonoMethodHandle get_scene_index_method;
	static MonoFieldHandle get_entity_id_field;
	static MonoClassHandle game_object_class;
	static MonoMethodHandle create_game_object_method;
	static MonoMethodHandle new_game_object_with_existing_entity_method;

public:
	static void RegisterInterface(CSMonoCore* mono_core);

public:
	static Entity GetEntityID(const CSMonoObject& game_object);
	static SceneIndex GetSceneIndex(const CSMonoObject& game_object);

	static CSMonoObject CreateGameObject();
	static CSMonoObject NewGameObjectWithExistingEntity(Entity entity, SceneIndex scene_index);
};

