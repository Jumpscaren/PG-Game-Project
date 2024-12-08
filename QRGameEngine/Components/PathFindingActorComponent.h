#pragma once
#include "ECS/EntityDefinition.h"
#include "Scripting/CSMonoObject.h"
#include "SceneSystem/SceneDefines.h"
#include "Common/EngineTypes.h"

struct PathFindingActorComponent
{
	Entity last_visited_node = NULL_ENTITY;
	Entity goal_last_visited_node = NULL_ENTITY;
	uint8_t last_path_index;
	std::vector<Entity> cached_path;
	qr::unordered_set<Entity> cached_mapped_path;
};

class JsonObject;
class EntityManager;

class PathFindingActorComponentInterface
{
public:
	static void RegisterInterface(CSMonoCore* mono_core);
	static void InitComponent(const CSMonoObject& object, SceneIndex scene_index, Entity entity);
	static bool HasComponent(const CSMonoObject& object, SceneIndex scene_index, Entity entity);
	static void RemoveComponent(const CSMonoObject& object, SceneIndex scene_index, Entity entity);

public:
	static CSMonoObject PathFind(const SceneIndex actor_scene_index, const Entity actor_entity, const SceneIndex goal_scene_index, const Entity goal_entity, const uint32_t position_of_node_index);
	static void DebugPath(const CSMonoObject& object);
	static bool NeedNewPathFind(const SceneIndex actor_scene_index, const Entity actor_entity, const SceneIndex goal_scene_index, const Entity goal_entity, const uint32_t position_of_node_index);

private:
	static bool HasToPathFind(const PathFindingActorComponent& path_finding_actor, const Entity own_node, const Entity goal_node);
	static void GetRandomNodes(const CSMonoObject& game_object, const CSMonoObject& list, const uint32_t number_of_nodes);

public:
	static void SavePathFindingWorldComponent(Entity ent, EntityManager* entman, JsonObject* json_object);
	static void LoadPathFindingWorldComponent(Entity ent, EntityManager* entman, JsonObject* json_object);
};

