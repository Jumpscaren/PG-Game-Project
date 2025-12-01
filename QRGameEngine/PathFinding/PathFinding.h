#pragma once
#include "ECS/EntityDefinition.h"
#include "SceneSystem/SceneDefines.h"
#include "Components/PathFindingWorldComponent.h"
#include "Common/EngineTypes.h"
#include "PathFindingDefines.h"
#include <thread>
#include <mutex>

struct PathFindingActorComponent;

class PathFinding
{
public:
	std::size_t GetNodeDetailIndex(const NodeDetail node_detail) const { return static_cast<std::size_t>(node_detail); }

private:
	static constexpr float BASE_WEIGHT = 1.0f;
	static constexpr float EXPENSIVE_WEIGHT = 10.0f;

	struct Node {
		NodeIndex node_index;
		std::vector<NodeIndex> neighbors;

		NodeIndex parent_node_index = NULL_NODE_INDEX;
		std::vector<NodeIndex> sub_nodes;

		Vector2 position;
		PathFindingWorldComponent layer;
		float weight = BASE_WEIGHT;

		Node(const NodeIndex node_index, const PathFindingWorldComponent& layer, const Vector2& position, const NodeIndex parent_node_index = NULL_NODE_INDEX) : node_index(node_index), position(position), parent_node_index(parent_node_index), layer(layer)  {}
	};

	struct SceneEntityData
	{
		SceneIndex scene_index;
		Entity entity;

		bool operator==(const SceneEntityData& other) const { return scene_index == other.scene_index && entity == other.entity; }
		bool operator!=(const SceneEntityData& other) const { return !(*this == other); }
	};

	struct SceneEntityDataHasher
	{
		std::size_t operator()(const SceneEntityData scene_entity_data) const
		{
			std::size_t res = 17;
			res = res * 31 + std::hash<uint32_t>()(scene_entity_data.entity);
			res = res * 31 + std::hash<uint32_t>()(scene_entity_data.scene_index);
			return res;
		}
	};

	struct PathFindData
	{
		SceneIndex scene_index;
		Entity entity;

		//Node Data
		NodeIndex start_node_index;
		NodeIndex goal_node_index;

		NodeDetail node_detail = NodeDetail::Base;
	};

	enum class ThreadState
	{
		Run = 0,
		Wait = 1,
		Close = 2
	};

	struct CalculatedPath
	{
		std::vector<NodeIndex> path;
		bool new_path = false;
	};

	struct ProcessedRequest 
	{
		PathFindData path_find_data;
		CalculatedPath calculated_path;
	};

private:
	static void AStarPathfinding(const PathFindData& path_find_data, std::vector<NodeIndex>& path, const std::vector<Node>& world);
	uint64_t PositionToNodeIndex(const Vector2& position, const NodeDetail node_detail) const;
	static float Heuristic(const Vector2 start_goal_direction, const Node& neighbor_node, const Node& goal_node);
	static void ConstructPath(const qr::unordered_map<NodeIndex, NodeIndex, NodeIndexHasher>& came_from, NodeIndex current, std::vector<NodeIndex>& path, const std::vector<Node>& world);
	Vector2 PositionToNodePosition(const Vector2& position, const NodeDetail node_detail) const;
	static void ConstructPathFindingWorldEvent(const SceneIndex scene_index);
	void ConstructPathFindingWorld(const SceneIndex scene_index);

	void CalculatePath(const PathFindData& path_find_data, std::vector<NodeIndex>& path);
	void PresentPath(const PathFindingActorComponent& path_finding_actor);

	void ThreadHandleRequests();

	Node* AddNeighbor(Node& node, const Vector2& neighbor_position, const NodeDetail node_detail);
	void RemoveImpossibleCornerPaths(Node& node, const NodeDetail node_detail);
	void CheckCollisionWithColliders(Node& node, const NodeDetail node_detail, EntityManager* entity_manager);

	void AddNewNodeInternal(const SceneIndex scene_index, const Entity entity);
	NodeIndex AddNewNodeInternal(const Vector2& postion, const PathFindingWorldComponent& path_finding_world, const NodeDetail node_detail);
	void RemoveNodeInternal(const SceneIndex scene_index, const Entity entity);
	void RemoveNodeFromNeighbours(const NodeIndex node_index, const NodeDetail node_detail);

public:
	static constexpr float LENGTH_BETWEEN_NODES = 1.0f;

	static constexpr float SMALL_LENGTH_BETWEEN_NODES = LENGTH_BETWEEN_NODES / 2.0f;
	static constexpr float HALF_SMALL_LENGTH_BETWEEN_NODES = SMALL_LENGTH_BETWEEN_NODES / 2.0f;

private:
	static PathFinding* s_singleton;

	static constexpr NodeIndex START_NODE_INDEX = NodeIndex(0);
	NodeIndex m_next_node_index = START_NODE_INDEX;
	struct NodeDataByNodeDetail
	{
		qr::unordered_map<uint64_t, NodeIndex> position_to_node;
		qr::unordered_set<NodeIndex, NodeIndexHasher> node_indices;
	};
	std::vector<NodeDataByNodeDetail> m_nodes_by_node_detail;
	std::vector<Node> m_nodes;
	std::vector<NodeIndex> m_removed_node_indices;
	qr::unordered_map<NodeIndex, Entity, NodeIndexHasher> m_node_to_entity;

	static constexpr float WEIGHT_BETWEEN_NODES = 1.0f;
	static constexpr float HALF_LENGTH_BETWEEN_NODES = LENGTH_BETWEEN_NODES / 2.0f;
	static constexpr size_t MAX_NEIGHBORS = 8;

	//
	std::vector<PathFindData> m_wait_list_paths_to_be_processed;
	qr::unordered_map<SceneEntityData, CalculatedPath, SceneEntityDataHasher> m_calculated_paths;
	std::vector<SceneEntityData> m_nodes_to_be_added;
	std::vector<SceneEntityData> m_nodes_to_be_removed;

	std::thread* m_calculate_paths_thread = nullptr;
	std::mutex m_calculate_paths_mutex;
	std::vector<PathFindData> m_paths_wait_list;
	std::vector<ProcessedRequest> m_paths_calculated_to_be_processed;
	qr::unordered_set<SceneEntityData, SceneEntityDataHasher> m_entity_ongoing_pathfinds;
	std::atomic<ThreadState> m_thread_state;

public:
	PathFinding();
	~PathFinding();
	bool RequestPathFind(const SceneIndex scene_index, const Entity start_entity, const Entity goal_entity, const NodeDetail node_detail);
	std::vector<NodeIndex> PathFind(const SceneIndex scene_index, const Entity start_entity, const Entity goal_entity, bool& new_path, const NodeDetail node_detail);
	std::vector<NodeIndex> GetPath(const SceneIndex scene_index, const Entity entity);
	NodeIndex GetNodeFromPosition(const Vector2& position, const NodeDetail node_detail) const;
	Entity GetEntityFromNodeIndex(const NodeIndex node_index) const;
	bool IsWorldConstructed() const;
	Vector2 GetNodePosition(const NodeIndex node_index) const;

	void HandleDeferredRemovedNodes(EntityManager* entity_manager);

	void HandlePathFinding();

	void AddNewNode(const SceneIndex scene_index, const Entity entity);
	void RemoveNode(const SceneIndex scene_index, const Entity entity);

	static PathFinding* Get();
};

