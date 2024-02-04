#pragma once
#include "ECS/EntityDefinition.h"
#include "SceneSystem/SceneDefines.h"
#include "Components/PathFindingWorldComponent.h"
#include "Common/EngineTypes.h"

class PathFinding
{
private:
	struct Node {
		Entity entity;
		float f_score;
		std::vector<Entity> neighbors;

		Vector2 position;
		PathFindingWorldComponent layer;

		Node(const Entity entity, const PathFindingWorldComponent& layer, const Vector2& position) : entity(entity), position(position), layer(layer) { f_score = 0; }
	};

private:
	void AStarPathfinding(const SceneIndex scene_index, const Entity start_entity, const Entity goal_entity, std::vector<Entity>& pat);
	uint64_t PositionToNodeIndex(const Vector2& position) const;
	float Heuristic(const Node& current_node, const Node& goal_node);
	void ConstructPath(const std::map<Entity, Entity>& came_from, Entity current, std::vector<Entity>& path);
	Vector2 PositionToNodePosition(const Vector2& position) const;
	static void ConstructPathFindingWorldEvent(const SceneIndex scene_index);
	void ConstructPathFindingWorld(const SceneIndex scene_index);

private:
	static PathFinding* s_singleton;
	std::unordered_map<Entity, Node> m_nodes;
	std::unordered_map<uint64_t, Entity> m_position_to_node;

	static constexpr float WEIGHT_BETWEEN_NODES = 1.0f;
	static constexpr float LENGTH_BETWEEN_NODES = 1.0f;
	static constexpr float HALF_LENGTH_BETWEEN_NODES = LENGTH_BETWEEN_NODES / 2.0f;
	static constexpr size_t MAX_NEIGHBORS = 8;

public:
	PathFinding();
	std::vector<Entity> PathFind(const SceneIndex scene_index, const Entity start_entity, const Entity goal_entity);
	Entity GetNodeFromPosition(const Vector2& position) const;

	static PathFinding* Get();
};

