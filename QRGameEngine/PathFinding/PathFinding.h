#pragma once
#include "ECS/EntityDefinition.h"
#include "SceneSystem/SceneDefines.h"
#include "Components/PathFindingWorldComponent.h"
#include "Common/EngineTypes.h"
#include <thread>
#include <mutex>

class PathFinding
{
private:
	struct Node {
		Entity entity;
		std::vector<Entity> neighbors;

		Vector2 position;
		PathFindingWorldComponent layer;

		Node(const Entity entity, const PathFindingWorldComponent& layer, const Vector2& position) : entity(entity), position(position), layer(layer) { }
	};

	struct PathFindData
	{
		SceneIndex scene_index;
		Entity entity;

		//Node Data
		Entity start_node_index;
		Entity goal_node_index;
	};

	enum class ThreadState
	{
		Run = 0,
		Clear = 1,
		Close = 2
	};

	struct CalculatedPath
	{
		std::vector<Entity> path;
		bool new_path = false;
	};

private:
	void AStarPathfinding(const PathFindData& path_find_data, std::vector<Entity>& path);
	uint64_t PositionToNodeIndex(const Vector2& position) const;
	float Heuristic(const Node& neighbor_node, const Node& goal_node);
	void ConstructPath(const qr::unordered_map<Entity, Entity>& came_from, Entity current, std::vector<Entity>& path);
	Vector2 PositionToNodePosition(const Vector2& position) const;
	static void ConstructPathFindingWorldEvent(const SceneIndex scene_index);
	void ConstructPathFindingWorld(const SceneIndex scene_index);

	void CalculatePath(const PathFindData& path_find_data, std::vector<Entity>& path);

	void ThreadHandleRequests();

	Node* AddNeighbor(Node& node, const Vector2& neighbor_position);
	void RemoveImpossibleCornerPaths(Node& node, EntityManager* entity_manager);

public:
	static constexpr float LENGTH_BETWEEN_NODES = 1.0f;

private:
	static PathFinding* s_singleton;
	qr::unordered_map<Entity, Node> m_nodes;
	qr::unordered_map<uint64_t, Entity> m_position_to_node;

	static constexpr float WEIGHT_BETWEEN_NODES = 1.0f;
	static constexpr float HALF_LENGTH_BETWEEN_NODES = LENGTH_BETWEEN_NODES / 2.0f;
	static constexpr size_t MAX_NEIGHBORS = 8;

	std::thread* m_calculate_paths_thread = nullptr;
	std::mutex m_calculate_paths_mutex;
	std::mutex m_clear_data_mutex;
	std::vector<PathFindData> m_paths_wait_list;
	std::vector<PathFindData> m_paths_calculating;
	qr::unordered_set<Entity> m_entity_ongoing_pathfinds;
	qr::unordered_map<Entity, CalculatedPath> m_calculated_paths;
	std::atomic<ThreadState> m_thread_state;

public:
	PathFinding();
	~PathFinding();
	void RequestPathFind(const SceneIndex scene_index, const Entity start_entity, const Entity goal_entity);
	std::vector<Entity> PathFind(const SceneIndex scene_index, const Entity start_entity, const Entity goal_entity, bool& new_path);
	Entity GetNodeFromPosition(const Vector2& position) const;
	bool IsWorldConstructed() const;
	void AddNewNode(const SceneIndex scene_index, const Entity entity);
	void RemoveNode(const SceneIndex scene_index, const Entity entity);
	const qr::unordered_map<uint64_t, Entity>& GetPositionsToNode() const;

	void HandleDeferredRemovedNodes(EntityManager* entity_manager);

	static PathFinding* Get();
};

