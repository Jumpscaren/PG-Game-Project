#pragma once
#include "Scripting/CSMonoObject.h"
#include "SceneSystem/SceneDefines.h"
#include "ECS/EntityDefinition.h"

class EntityManager;

class GameObjectInterface
{
private:
	static MonoMethodHandle get_scene_index_method;
	static MonoFieldHandle get_entity_id_field;
	static MonoClassHandle game_object_class;
	static MonoMethodHandle create_game_object_method;
	static MonoMethodHandle new_game_object_with_existing_entity_method;
	static MonoMethodHandle remove_scene_from_scene_to_component_map_method;
	static MonoMethodHandle remove_entity_from_scene_to_component_map_method;
	static MonoMethodHandle remove_game_object_from_database_method;

private:
	static void RemoveSceneFromSceneToComponentMap(SceneIndex scene_index);
	static void RemoveEntityFromSceneToComponentMap(SceneIndex scene_index, Entity entity);
	static void RemoveGameObjectFromDatabase(SceneIndex scene_index, Entity entity);

public:
	static void RegisterInterface(CSMonoCore* mono_core);

public:
	static CSMonoObject GetGameObjectFromComponent(const CSMonoObject& component);
	static Entity GetEntityID(const CSMonoObject& game_object);
	static SceneIndex GetSceneIndex(const CSMonoObject& game_object);

	static CSMonoObject CreateGameObject();
	static CSMonoObject NewGameObjectWithExistingEntity(Entity entity, SceneIndex scene_index);

	static void AddEntityData(const CSMonoObject& object);

public:
	static void SetName(const CSMonoObject& object, const std::string& name);
	static std::string GetName(const CSMonoObject& object);

	static CSMonoObject TempFindGameObject(const std::string& name);
	static Entity TempFindGameObjectEntity(const std::string& name);
	static CSMonoObject FindGameObjectWithTag(const uint8_t tag);
	static void FindGameObjectsWithTag(const CSMonoObject& list, const uint8_t tag);

	static void AddChild(const CSMonoObject& game_object, const CSMonoObject& child_game_object);
	static void RemoveChild(const CSMonoObject& game_object, const CSMonoObject& child_game_object);
	static bool HasChildren(const CSMonoObject& game_object);
	static void DestroyChildren(const CSMonoObject& game_object);
	static CSMonoObject GetParent(const SceneIndex scene_index, const Entity entity);
	
	static void SetTag(const CSMonoObject& game_object, const uint8_t tag);
	static uint8_t GetTag(const CSMonoObject& game_object);

	static void HandleDeferredEntities(EntityManager* const entity_manager);
};

