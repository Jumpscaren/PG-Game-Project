#include "pch.h"
#include "PathFinding.h"
#include "Components/TransformComponent.h"
#include "Components/PathFindingWorldComponent.h"
#include <queue>
#include "ECS/EntityManager.h"
#include "SceneSystem/SceneManager.h"
#include "Event/EventCore.h"

PathFinding* PathFinding::s_singleton = nullptr;

float PathFinding::Heuristic(const Node& current_node, const Node& goal_node)
{
	Vector2 diff = current_node.position - goal_node.position;
	return diff.Length() / LENGTH_BETWEEN_NODES;
}

void PathFinding::ConstructPath(const std::map<Entity, Entity>& came_from, Entity current, std::vector<Entity>& path)
{
	while (came_from.contains(current))
	{
		current = came_from.find(current)->second;
		path.push_back(current);
	}
}

Vector2 PathFinding::PositionToNodePosition(const Vector2& position) const
{
	Vector2 node_position;
	node_position.x = float(int(position.x)) + (position.x < 0.0f ? -HALF_LENGTH_BETWEEN_NODES : HALF_LENGTH_BETWEEN_NODES);
	node_position.y = float(int(position.y)) + (position.y < 0.0f ? -HALF_LENGTH_BETWEEN_NODES : HALF_LENGTH_BETWEEN_NODES);
	return node_position;
}

void PathFinding::ConstructPathFindingWorldEvent(const SceneIndex scene_index)
{
	s_singleton->ConstructPathFindingWorld(scene_index);
}

void PathFinding::ConstructPathFindingWorld(const SceneIndex scene_index)
{
	m_nodes.clear();
	m_position_to_node.clear();

	EntityManager* entity_manager = SceneManager::GetEntityManager(scene_index);
	entity_manager->System<TransformComponent, PathFindingWorldComponent>([&](const Entity entity, const TransformComponent& transform, const PathFindingWorldComponent& path_finding_world)
		{
			const Vector3 transform_position = transform.GetPosition();
			Vector2 position(transform_position.x, transform_position.y);

			m_position_to_node.insert({PositionToNodeIndex(position), entity});

			m_nodes.insert({ entity, Node(entity, path_finding_world, position) });
		});

	const auto length_between_nodes = std::lround(LENGTH_BETWEEN_NODES);
	for (auto& node : m_nodes)
	{
		for (const auto& potential_neighbor : m_nodes)
		{
			Vector2 diff = node.second.position - potential_neighbor.second.position;
			if (std::lround(diff.Length()) == length_between_nodes)
			{
				node.second.neighbors.push_back(potential_neighbor.second.entity);
			}

			if (node.second.neighbors.size() == MAX_NEIGHBORS)
			{
				break;
			}
		}
	}
}

void PathFinding::AStarPathfinding(const SceneIndex scene_index, const Entity start_entity, const Entity goal_entity, std::vector<Entity>& path)
{
	EntityManager* entity_manager = SceneManager::GetEntityManager(scene_index);
	const TransformComponent& ts = entity_manager->GetComponent<TransformComponent>(start_entity);
	const TransformComponent& tg = entity_manager->GetComponent<TransformComponent>(goal_entity);
	const Vector2 start_position(ts.GetPosition().x, ts.GetPosition().y);
	const Vector2 goal_position(tg.GetPosition().x, tg.GetPosition().y);

	const auto found_start_node = m_position_to_node.find(PositionToNodeIndex(start_position));
	const auto found_goal_node = m_position_to_node.find(PositionToNodeIndex(goal_position));
	if (found_start_node == m_position_to_node.end() || found_goal_node == m_position_to_node.end())
	{
		const auto test_found_start_node = m_position_to_node.find(PositionToNodeIndex(start_position));
		const auto test_found_goal_node = m_position_to_node.find(PositionToNodeIndex(goal_position));
		return;
	}

	Entity start_node_index = found_start_node->second;
	Entity goal_node_index = found_goal_node->second;
	std::swap(start_node_index, goal_node_index);

	const auto cmp = [](const Node& left, const Node& right) { return left.f_score > right.f_score; };
	std::priority_queue<Node, std::vector<Node>, decltype(cmp)> open_set(cmp);
	std::set<Entity> open_set_entities;

	auto start_node = m_nodes.at(start_node_index);
	const auto& goal_node = m_nodes.at(goal_node_index);
	start_node.f_score = Heuristic(start_node, goal_node);
	open_set.emplace(m_nodes.at(start_node_index));
	open_set_entities.insert(start_node_index);
	std::map<Entity, Entity> came_from;
	std::map<Entity, float> g_score;

	g_score.insert({ start_node_index, 0.0f });


	Entity previous = 0;
	while (open_set.size() > 0)
	{
		const auto current = open_set.top();
		open_set.pop();
		open_set_entities.erase(current.entity);

		if (current.entity == goal_node.entity)
		{
			path.push_back(current.entity);
			ConstructPath(came_from, current.entity, path);
			return;
		}
		previous = current.entity;

		for (const auto& neighbor : current.neighbors)
		{
			auto g_score_neighbor_it = g_score.find(neighbor);
			if (g_score_neighbor_it == g_score.end())
			{
				g_score.insert({neighbor, FLT_MAX});
				g_score_neighbor_it = g_score.find(neighbor);
			}
			auto& neighbor_node = m_nodes.at(neighbor);
			const Vector2 diff = neighbor_node.position - current.position;
			const auto tentative_g_score = g_score.at(current.entity) + diff.Length();
			if (tentative_g_score < g_score_neighbor_it->second)
			{
				came_from.insert({ neighbor, current.entity });
				g_score_neighbor_it->second = tentative_g_score;

				//Neighbor not in open_set
				if (!open_set_entities.contains(neighbor))
				{
					neighbor_node.f_score = tentative_g_score + Heuristic(neighbor_node, goal_node);
					open_set.push(neighbor_node);
					open_set_entities.insert(neighbor);
				}
			}
		}
	}
}

uint64_t PathFinding::PositionToNodeIndex(const Vector2& position) const
{
	Vector2 node_position = PositionToNodePosition(position);
	uint64_t index = 0;
	uint8_t* ptr_un = (uint8_t*)&index;
	*((uint32_t*)(ptr_un)) = (uint32_t)(node_position.x / HALF_LENGTH_BETWEEN_NODES);
	ptr_un += sizeof(uint32_t);
	*((uint32_t*)(ptr_un)) = (uint32_t)(node_position.y / HALF_LENGTH_BETWEEN_NODES);

	return index;
}

PathFinding::PathFinding()
{
	s_singleton = this;
	EventCore::Get()->ListenToEvent<PathFinding::ConstructPathFindingWorldEvent>("SceneLoaded", 0, PathFinding::ConstructPathFindingWorldEvent);
}

std::vector<Entity> PathFinding::PathFind(const SceneIndex scene_index, const Entity start_entity, const Entity goal_entity)
{
	std::vector<Entity> path;
	AStarPathfinding(scene_index, start_entity, goal_entity, path);
	return path;
}

Entity PathFinding::GetNodeFromPosition(const Vector2& position) const
{
	const auto it = m_position_to_node.find(PositionToNodeIndex(position));
	if (it != m_position_to_node.end())
	{
		return it->second;
	}
	return NULL_ENTITY;
}

PathFinding* PathFinding::Get()
{
	return s_singleton;
}
