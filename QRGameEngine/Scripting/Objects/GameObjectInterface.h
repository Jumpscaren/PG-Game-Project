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
	static CSMonoObject GetGameObjectFromComponent(const CSMonoObject& component);
	static Entity GetEntityID(const CSMonoObject& game_object);
	static SceneIndex GetSceneIndex(const CSMonoObject& game_object);

	static CSMonoObject CreateGameObject();
	static CSMonoObject NewGameObjectWithExistingEntity(Entity entity, SceneIndex scene_index);

	static void AddEntityData(CSMonoObject object);

public:
	static void SetName(CSMonoObject object, std::string name);
	static std::string GetName(CSMonoObject object);

	static CSMonoObject TempFindGameObject(std::string name);
	static Entity TempFindGameObjectEntity(const std::string& name);

	static void AddChild(const CSMonoObject game_object, const CSMonoObject child_game_object);
	static void RemoveChild(const CSMonoObject game_object, const CSMonoObject child_game_object);
};

