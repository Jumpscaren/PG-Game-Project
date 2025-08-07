#pragma once
#include "ECS/EntityDefinition.h"
#include "Scripting/CSMonoObject.h"
#include "SceneSystem/SceneDefines.h"
#include "Common/EngineTypes.h"
#include "PathFinding/PathFindingDefines.h"

struct PathFindingActorComponent
{
	NodeIndex last_visited_node = NULL_NODE_INDEX;
	NodeIndex goal_last_visited_node = NULL_NODE_INDEX;
	uint8_t last_path_index;
	std::vector<NodeIndex> cached_path;
	//qr::unordered_set<NodeIndex, NodeIndexHasher> cached_mapped_path;
	qr::unordered_set<NodeIndex, NodeIndexHasher> cached_base_nodes;
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
	static bool IsPositionInPath(const CSMonoObject& object, const CSMonoObject& position);
	static bool NeedNewPathFind(const SceneIndex actor_scene_index, const Entity actor_entity, const SceneIndex goal_scene_index, const Entity goal_entity, const uint32_t position_of_node_index);

	static CSMonoObject GetGameObjectNodeByPosition(const CSMonoObject& position);

private:
	static bool HasToPathFind(const PathFindingActorComponent& path_finding_actor, const NodeIndex own_node, const NodeIndex goal_node);
	static bool IsPositionInWorld(const CSMonoObject& position);
	static void GetRandomNodes(const CSMonoObject& game_object, const CSMonoObject& list, const uint32_t number_of_nodes);

public:
	static void SavePathFindingWorldComponent(Entity ent, EntityManager* entman, JsonObject* json_object);
	static void LoadPathFindingWorldComponent(Entity ent, EntityManager* entman, JsonObject* json_object);
};

