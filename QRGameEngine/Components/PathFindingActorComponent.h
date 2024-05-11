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
	std::unordered_set<Entity> cached_mapped_path;
};

class JsonObject;
class EntityManager;

class PathFindingActorComponentInterface
{
public:
	static void RegisterInterface(CSMonoCore* mono_core);
	static void InitComponent(CSMonoObject object, SceneIndex scene_index, Entity entity);
	static bool HasComponent(CSMonoObject object, SceneIndex scene_index, Entity entity);
	static void RemoveComponent(CSMonoObject object, SceneIndex scene_index, Entity entity);

public:
	static CSMonoObject PathFind(const CSMonoObject object, const CSMonoObject goal_game_object, uint32_t position_of_node_index);
	static void DebugPath(const CSMonoObject object);
	static bool NeedNewPathFind(const CSMonoObject object, const CSMonoObject goal_game_object, const uint32_t position_of_node_index);

private:
	static bool HasToPathFind(const PathFindingActorComponent& path_finding_actor, const Entity goal_node);

public:
	static void SavePathFindingWorldComponent(Entity ent, EntityManager* entman, JsonObject* json_object);
	static void LoadPathFindingWorldComponent(Entity ent, EntityManager* entman, JsonObject* json_object);
};

