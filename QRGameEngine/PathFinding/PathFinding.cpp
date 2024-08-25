#include "pch.h"
#include "PathFinding.h"
#include "Components/TransformComponent.h"
#include "Components/PathFindingWorldComponent.h"
#include <queue>
#include "ECS/EntityManager.h"
#include "SceneSystem/SceneManager.h"
#include "Event/EventCore.h"

PathFinding* PathFinding::s_singleton = nullptr;

float PathFinding::Heuristic(const Node& neighbor_node, const Node& goal_node)
{
	return 0;
	const Vector2 diff = goal_node.position - neighbor_node.position;
	Vector2 dir = diff.Normalize();
	if (dir.x < 0)
	{
		dir.x *= -2.0f;
	}
	if (dir.y < 0)
	{
		dir.y *= -2.0f;
	}
	return dir.Length();
	//return std::roundf(diff.Length() / LENGTH_BETWEEN_NODES);
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
	m_thread_state = ThreadState::Clear;
	m_clear_data_mutex.lock();

	m_nodes.clear();
	m_position_to_node.clear();
	m_calculated_paths.clear();
	m_entity_ongoing_pathfinds.clear();
	m_paths_calculating.clear();
	m_paths_wait_list.clear();

	m_clear_data_mutex.unlock();
	m_thread_state = ThreadState::Run;

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
			const Vector2 diff = node.second.position - potential_neighbor.second.position;
			if (std::lround(diff.Length()) == length_between_nodes)
			{
				//if (!(std::abs(diff.x) > 1e-05f && std::abs(diff.y) > 1e-05f))
				{
					node.second.neighbors.push_back(potential_neighbor.second.entity);
				}
			}

			if (node.second.neighbors.size() == MAX_NEIGHBORS)
			{
				break;
			}
		}

		RemoveImpossibleCornerPaths(node.second, entity_manager);
	}
}

void PathFinding::CalculatePath(const PathFindData& path_find_data, std::vector<Entity>& path)
{
	AStarPathfinding(path_find_data, path);
}

void PathFinding::ThreadHandleRequests()
{
	m_clear_data_mutex.lock();
	while (true)
	{
		if (m_thread_state == ThreadState::Close)
		{
			break;
		}

		if (m_thread_state == ThreadState::Clear)
		{
			m_clear_data_mutex.unlock();
			while (m_thread_state != ThreadState::Run)
			{

			}
			m_clear_data_mutex.lock();
		}

		Entity remove_from_ongoing_entity = NULL_ENTITY;
		std::vector<Entity> path;
		if (m_paths_calculating.size() > 0)
		{
			const auto path_find_data = m_paths_calculating.front();
			m_paths_calculating.erase(m_paths_calculating.begin());
			AStarPathfinding(path_find_data, path);
			remove_from_ongoing_entity = path_find_data.entity;
		}

		m_calculate_paths_mutex.lock();
		for (const auto& data : m_paths_wait_list)
		{
			m_paths_calculating.push_back(data);
		}
		m_paths_wait_list.clear();
		if (remove_from_ongoing_entity != NULL_ENTITY)
		{
			m_entity_ongoing_pathfinds.erase(remove_from_ongoing_entity);
			auto it = m_calculated_paths.find(remove_from_ongoing_entity);
			it->second.path = path;
			it->second.new_path = true;
		}
		m_calculate_paths_mutex.unlock();
	}
	m_clear_data_mutex.unlock();
}

PathFinding::Node* PathFinding::AddNeighbor(Node& node, const Vector2& neighbor_position)
{
	const auto neighbor_node = GetNodeFromPosition(neighbor_position);
	if (auto it = m_nodes.find(neighbor_node); it != m_nodes.end())
	{
		it->second.neighbors.push_back(node.entity);
		node.neighbors.push_back(it->first);
		return &(it->second);
	}
	return nullptr;
}

void PathFinding::RemoveImpossibleCornerPaths(Node& node, EntityManager* entity_manager)
{
	//We want to avoid the case when a path goes between a empty place and a node
		/*
		* Example
		* A B
		* C D E
		* The most optimal path from B to E is a diagonal path, but in this case we want to avoid that the enemies get stuck on walls so we want them to go B-D-E.
		*/
	const auto length_between_nodes = std::lround(LENGTH_BETWEEN_NODES);
	for (size_t i{ 0 }; i < node.neighbors.size(); ++i)
	{
		const auto neighbor = node.neighbors[i];
		const Vector3 neighbor_position_v3 = entity_manager->GetComponent<TransformComponent>(neighbor).GetPosition();
		const Vector2 neighbor_position(neighbor_position_v3.x, neighbor_position_v3.y);
		const Vector2 diff = node.position - neighbor_position;
		if (std::abs(diff.x) > 1e-05f && std::abs(diff.y) > 1e-05f)
		{
			char found_neighbors = 0;
			for (const auto compare_node : node.neighbors)
			{
				const Vector3 compare_node_position_v3 = entity_manager->GetComponent<TransformComponent>(compare_node).GetPosition();
				const Vector2 compare_node_position(compare_node_position_v3.x, compare_node_position_v3.y);
				const Vector2 compare_diff = node.position - compare_node_position;
				if ((std::abs(compare_diff.x) > 1e-05f && std::abs(compare_diff.y) < 1e-05f) || (std::abs(compare_diff.x) < 1e-05f && std::abs(compare_diff.y) > 1e-05f))
				{
					const Vector2 compare_neighbor_diff = neighbor_position - compare_node_position;
					if (std::lround(compare_neighbor_diff.Length()) != length_between_nodes)
					{
						continue;
					}
					if (std::abs(compare_neighbor_diff.x) > 1e-05f)
					{
						++found_neighbors;
					}
					else if (std::abs(compare_neighbor_diff.y) > 1e-05f)
					{
						++found_neighbors;
					}
				}
				if (found_neighbors == 2)
				{
					break;
				}
			}
			if (found_neighbors != 2)
			{
				node.neighbors.erase(node.neighbors.begin() + i);
				--i;
			}
		}
	}
}

void PathFinding::AStarPathfinding(const PathFindData& path_find_data, std::vector<Entity>& path)
{
	const auto start_node_index = path_find_data.start_node_index;
	const auto goal_node_index = path_find_data.goal_node_index;

	std::map<Entity, float> f_score;
	const auto cmp = [&](Node* left, Node* right) { return f_score.at(left->entity) > f_score.at(right->entity); };
	std::priority_queue<Node*, std::vector<Node*>, decltype(cmp)> open_set(cmp);
	std::set<Entity> open_set_entities;

	auto& start_node = m_nodes.at(start_node_index);
	const auto& goal_node = m_nodes.at(goal_node_index);
	open_set.emplace(&(m_nodes.at(start_node_index)));
	std::map<Entity, Entity> came_from;
	std::map<Entity, float> g_score;

	g_score.insert({ start_node_index, 0.0f });
	f_score.insert({ start_node_index, Heuristic(start_node, goal_node) });

	Entity previous = 0;
	while (open_set.size() > 0)
	{
		const auto current = open_set.top();
		open_set.pop();
		open_set_entities.erase(current->entity);

		if (current->entity == goal_node.entity)
		{
			path.push_back(current->entity);
			ConstructPath(came_from, current->entity, path);
			return;
		}
		previous = current->entity;

		for (const auto& neighbor : current->neighbors)
		{
			auto g_score_neighbor_it = g_score.find(neighbor);
			if (g_score_neighbor_it == g_score.end())
			{
				g_score.insert({neighbor, FLT_MAX});
				g_score_neighbor_it = g_score.find(neighbor);
			}
			auto& neighbor_node = m_nodes.at(neighbor);
			const Vector2 diff = neighbor_node.position - current->position;
			const auto tentative_g_score = g_score.at(current->entity) + diff.Length();
			if (tentative_g_score < g_score_neighbor_it->second)
			{
				came_from.insert({ neighbor, current->entity });
				g_score_neighbor_it->second = tentative_g_score;
				auto f_score_neighbor_it = f_score.find(neighbor);
				if (f_score_neighbor_it == f_score.end())
				{
					f_score.insert({ neighbor, FLT_MAX });
					f_score_neighbor_it = f_score.find(neighbor);
				}
				f_score_neighbor_it->second = tentative_g_score + Heuristic(neighbor_node, goal_node);

				//Neighbor not in open_set
				if (!open_set_entities.contains(neighbor))
				{
					open_set.push(&neighbor_node);
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

	m_thread_state = ThreadState::Run;
	m_calculate_paths_thread = new std::thread(&PathFinding::ThreadHandleRequests, this);
}

PathFinding::~PathFinding()
{
	m_thread_state = ThreadState::Close;
	m_calculate_paths_thread->join();
	delete m_calculate_paths_thread;
}

void PathFinding::RequestPathFind(const SceneIndex scene_index, const Entity start_entity, const Entity goal_entity)
{
	if (m_entity_ongoing_pathfinds.contains(start_entity))
	{
		return;
	}

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

	m_paths_wait_list.push_back(PathFindData{ .scene_index = scene_index, .entity = start_entity, .start_node_index = start_node_index, .goal_node_index = goal_node_index });
	m_entity_ongoing_pathfinds.insert(start_entity);
	m_calculated_paths.insert({ start_entity, CalculatedPath{.path = {}, .new_path = false} });
}

std::vector<Entity> PathFinding::PathFind(const SceneIndex scene_index, const Entity start_entity, const Entity goal_entity, bool& new_path)
{
	std::vector<Entity> path;
	m_calculate_paths_mutex.lock();
	RequestPathFind(scene_index, start_entity, goal_entity);
	auto it = m_calculated_paths.find(start_entity);
	new_path = it->second.new_path;
	it->second.new_path = false;
	path = it->second.path;
	m_calculate_paths_mutex.unlock();
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

bool PathFinding::IsWorldConstructed() const
{
	return m_nodes.size() > 0;
}

void PathFinding::AddNewNode(const SceneIndex scene_index, const Entity entity)
{
	m_thread_state = ThreadState::Clear;
	m_clear_data_mutex.lock();

	EntityManager* entity_manager = SceneManager::GetEntityManager(scene_index);
	const auto& transform_component = entity_manager->GetComponent<TransformComponent>(entity);
	const auto& path_finding_world = entity_manager->GetComponent<PathFindingWorldComponent>(entity);

	const Vector2 position(transform_component.GetPosition().x, transform_component.GetPosition().y);

	const Vector2 left_position(position.x - WEIGHT_BETWEEN_NODES, position.y);
	const Vector2 right_position(position.x + WEIGHT_BETWEEN_NODES, position.y);
	const Vector2 up_position(position.x, position.y + WEIGHT_BETWEEN_NODES);
	const Vector2 down_position(position.x, position.y - WEIGHT_BETWEEN_NODES);

	const Vector2 left_up_corner(position.x - WEIGHT_BETWEEN_NODES, position.y + WEIGHT_BETWEEN_NODES);
	const Vector2 left_down_corner(position.x - WEIGHT_BETWEEN_NODES, position.y - WEIGHT_BETWEEN_NODES);
	const Vector2 right_up_corner(position.x + WEIGHT_BETWEEN_NODES, position.y + WEIGHT_BETWEEN_NODES);
	const Vector2 right_down_corner(position.x + WEIGHT_BETWEEN_NODES, position.y - WEIGHT_BETWEEN_NODES);

	m_position_to_node.insert({ PositionToNodeIndex(position), entity });
	auto new_node = m_nodes.insert({ entity, Node(entity, path_finding_world, position) });

	std::vector<Node*> changed_nodes = { &(new_node.first->second) };

	changed_nodes.push_back(AddNeighbor(new_node.first->second, left_position));
	changed_nodes.push_back(AddNeighbor(new_node.first->second, right_position));
	changed_nodes.push_back(AddNeighbor(new_node.first->second, up_position));
	changed_nodes.push_back(AddNeighbor(new_node.first->second, down_position));

	changed_nodes.push_back(AddNeighbor(new_node.first->second, left_up_corner));
	changed_nodes.push_back(AddNeighbor(new_node.first->second, left_down_corner));
	changed_nodes.push_back(AddNeighbor(new_node.first->second, right_up_corner));
	changed_nodes.push_back(AddNeighbor(new_node.first->second, right_down_corner));

	for (Node* node : changed_nodes)
	{
		if (node)
		{
			RemoveImpossibleCornerPaths(*node, entity_manager);
		}
	}

	m_clear_data_mutex.unlock();
	m_thread_state = ThreadState::Run;
}

void PathFinding::RemoveNode(const SceneIndex scene_index, const Entity entity)
{
	m_thread_state = ThreadState::Clear;
	m_clear_data_mutex.lock();

	EntityManager* entity_manager = SceneManager::GetEntityManager(scene_index);
	const auto& transform_component = entity_manager->GetComponent<TransformComponent>(entity);
	const auto& path_finding_world = entity_manager->GetComponent<PathFindingWorldComponent>(entity);

	const Vector2 position(transform_component.GetPosition().x, transform_component.GetPosition().y);

	m_position_to_node.erase(PositionToNodeIndex(position));

	const Node& remove_node = m_nodes.at(entity);
	for (const Entity remove_node_neighbor_entity : remove_node.neighbors)
	{
		Node& remove_node_neighbor_node = m_nodes.at(remove_node_neighbor_entity);
		for (int i = 0; i < remove_node_neighbor_node.neighbors.size(); ++i)
		{
			if (remove_node_neighbor_node.neighbors[i] == entity)
			{
				remove_node_neighbor_node.neighbors.erase(remove_node_neighbor_node.neighbors.begin() + i);
				break;
			}
		}
		RemoveImpossibleCornerPaths(remove_node_neighbor_node, entity_manager);
	}

	m_nodes.erase(entity);

	m_clear_data_mutex.unlock();
	m_thread_state = ThreadState::Run;
}

const std::unordered_map<uint64_t, Entity>& PathFinding::GetPositionsToNode() const
{
	return m_position_to_node;
}

void PathFinding::HandleDeferredRemovedNodes(EntityManager* entity_manager)
{
	if (!IsWorldConstructed())
	{
		return;
	}

	entity_manager->System<DeferredEntityDeletion, PathFindingWorldComponent>([&](const Entity entity, DeferredEntityDeletion, PathFindingWorldComponent)
		{
			RemoveNode(entity_manager->GetSceneIndex(), entity);
		});
}

PathFinding* PathFinding::Get()
{
	return s_singleton;
}
