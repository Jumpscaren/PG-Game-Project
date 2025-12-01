#include "pch.h"
#include "PathFinding.h"
#include "Components/TransformComponent.h"
#include "Components/PathFindingWorldComponent.h"
#include "Components/PathFindingActorComponent.h"
#include <queue>
#include "ECS/EntityManager.h"
#include "SceneSystem/SceneManager.h"
#include "Event/EventCore.h"
#include "Renderer/RenderCore.h"
#include "Components/BoxColliderComponent.h"
#include "Components/PureStaticBodyComponent.h"

PathFinding* PathFinding::s_singleton = nullptr;

float PathFinding::Heuristic(const Vector2 start_goal_direction, const Node& neighbor_node, const Node& goal_node)
{
	const Vector2 diff = goal_node.position - neighbor_node.position;
	return diff.Length();
}

void PathFinding::ConstructPath(const qr::unordered_map<NodeIndex, NodeIndex, NodeIndexHasher>& came_from, NodeIndex current, std::vector<NodeIndex>& path, const std::vector<Node>& world)
{
	NodeIndex previous = NULL_NODE_INDEX;
	while (came_from.contains(current))
	{
		const NodeIndex next = came_from.find(current)->second;
		previous = current;
		current = next;
		path.push_back(current);
	}
}

Vector2 PathFinding::PositionToNodePosition(const Vector2& position, const NodeDetail node_detail) const
{
	float length = LENGTH_BETWEEN_NODES;
	float half_length = HALF_LENGTH_BETWEEN_NODES;
	if (node_detail == NodeDetail::Small)
	{
		length = SMALL_LENGTH_BETWEEN_NODES;
		half_length = HALF_SMALL_LENGTH_BETWEEN_NODES;
	}

	//position.x / length; 
	Vector2 node_position;
	const int x_multiplier = (int)(position.x / length);
	node_position.x = x_multiplier * length + (position.x < 0.0f ? -half_length : half_length);	
	const int y_multiplier = (int)(position.y / length);
	node_position.y = y_multiplier * length + (position.y < 0.0f ? -half_length : half_length);
	
	return node_position;
}

void PathFinding::ConstructPathFindingWorldEvent(const SceneIndex scene_index)
{
	s_singleton->ConstructPathFindingWorld(scene_index);
}

void PathFinding::ConstructPathFindingWorld(const SceneIndex scene_index)
{	
	m_thread_state = ThreadState::Wait;
	m_calculate_paths_mutex.lock();

	m_calculated_paths.clear();
	m_entity_ongoing_pathfinds.clear();
	m_paths_calculated_to_be_processed.clear();
	m_paths_wait_list.clear();
	m_wait_list_paths_to_be_processed.clear();

	m_nodes_by_node_detail.clear();
	m_node_to_entity.clear();
	m_nodes.clear();
	m_removed_node_indices.clear();
	m_nodes_to_be_added.clear();
	m_nodes_to_be_removed.clear();

	m_next_node_index = START_NODE_INDEX;

	m_calculate_paths_mutex.unlock();
	m_thread_state = ThreadState::Run;

	m_nodes_by_node_detail.push_back({}); // Large
	m_nodes_by_node_detail.push_back({}); // Base
	m_nodes_by_node_detail.push_back({}); // Small
	NodeDataByNodeDetail& node_data_by_node_detail = m_nodes_by_node_detail.at(GetNodeDetailIndex(NodeDetail::Base));

	EntityManager* entity_manager = SceneManager::GetEntityManager(scene_index);
	entity_manager->System<TransformComponent, PathFindingWorldComponent>([&](const Entity entity, const TransformComponent& transform, const PathFindingWorldComponent& path_finding_world)
		{
			const Vector3 transform_position = transform.GetPosition();
			Vector2 position(transform_position.x, transform_position.y);

			//m_nodes.insert({ entity, Node(entity, path_finding_world, position) });

			m_nodes.push_back(Node(m_next_node_index, path_finding_world, position));
			node_data_by_node_detail.node_indices.insert(m_next_node_index);
			m_node_to_entity.insert({ m_next_node_index, entity });
			node_data_by_node_detail.position_to_node.insert({ PositionToNodeIndex(position, NodeDetail::Base), m_next_node_index });

			m_next_node_index.increase();
		});

	const auto length_between_nodes = std::lround(LENGTH_BETWEEN_NODES);
	for (auto& node : m_nodes)
	{
		for (const auto& potential_neighbor : m_nodes)
		{
			const Vector2 diff = node.position - potential_neighbor.position;
			if (std::lround(diff.Length()) == length_between_nodes)
			{
				node.neighbors.push_back(potential_neighbor.node_index);
			}

			if (node.neighbors.size() == MAX_NEIGHBORS)
			{
				break;
			}
		}

		RemoveImpossibleCornerPaths(node, NodeDetail::Base);

		CheckCollisionWithColliders(node, NodeDetail::Base, entity_manager);
	}

	NodeDataByNodeDetail& small_nodes_by_node_detail = m_nodes_by_node_detail.at(GetNodeDetailIndex(NodeDetail::Small));
	for (const NodeIndex node_index : node_data_by_node_detail.node_indices)
	{
		Node& node = m_nodes[node_index.value];
		const Vector2 node_position = node.position;
		const PathFindingWorldComponent node_layer = node.layer;

		m_nodes.push_back(Node(m_next_node_index, node_layer, node_position + Vector2(-HALF_SMALL_LENGTH_BETWEEN_NODES, HALF_SMALL_LENGTH_BETWEEN_NODES), node_index));
		small_nodes_by_node_detail.node_indices.insert(m_next_node_index);
		small_nodes_by_node_detail.position_to_node.insert({ PositionToNodeIndex(m_nodes.back().position, NodeDetail::Small), m_next_node_index});
		m_nodes[node_index.value].sub_nodes.push_back(m_next_node_index);
		m_next_node_index.increase();

		m_nodes.push_back(Node(m_next_node_index, node_layer, node_position + Vector2(HALF_SMALL_LENGTH_BETWEEN_NODES, HALF_SMALL_LENGTH_BETWEEN_NODES), node_index));
		small_nodes_by_node_detail.node_indices.insert(m_next_node_index);
		small_nodes_by_node_detail.position_to_node.insert({ PositionToNodeIndex(m_nodes.back().position, NodeDetail::Small), m_next_node_index });
		m_nodes[node_index.value].sub_nodes.push_back(m_next_node_index);
		m_next_node_index.increase();

		m_nodes.push_back(Node(m_next_node_index, node_layer, node_position + Vector2(-HALF_SMALL_LENGTH_BETWEEN_NODES, -HALF_SMALL_LENGTH_BETWEEN_NODES), node_index));
		small_nodes_by_node_detail.node_indices.insert(m_next_node_index);
		small_nodes_by_node_detail.position_to_node.insert({ PositionToNodeIndex(m_nodes.back().position, NodeDetail::Small), m_next_node_index });
		m_nodes[node_index.value].sub_nodes.push_back(m_next_node_index);
		m_next_node_index.increase();

		m_nodes.push_back(Node(m_next_node_index, node_layer, node_position + Vector2(HALF_SMALL_LENGTH_BETWEEN_NODES, -HALF_SMALL_LENGTH_BETWEEN_NODES), node_index));
		small_nodes_by_node_detail.node_indices.insert(m_next_node_index);
		small_nodes_by_node_detail.position_to_node.insert({ PositionToNodeIndex(m_nodes.back().position, NodeDetail::Small), m_next_node_index });
		m_nodes[node_index.value].sub_nodes.push_back(m_next_node_index);
		m_next_node_index.increase();
	}

	constexpr float EPSILON = 1e-05f;
	const float HYP_SMALL_LENGTH_BETWEEN_NODES = std::sqrtf(2.0f) * SMALL_LENGTH_BETWEEN_NODES;
	for (const NodeIndex node_index : small_nodes_by_node_detail.node_indices)
	{
		Node& node = m_nodes[node_index.value];

		for (const NodeIndex potential_neighbor_index : small_nodes_by_node_detail.node_indices)
		{
			if (node_index == potential_neighbor_index)
			{
				continue;
			}

			Node& potential_neighbor = m_nodes[potential_neighbor_index.value];

			const Vector2 diff = node.position - potential_neighbor.position;
			const float diff_length = diff.Length();
			if (diff_length - EPSILON <= HYP_SMALL_LENGTH_BETWEEN_NODES)
			{
				node.neighbors.push_back(potential_neighbor.node_index);
			}

			if (node.neighbors.size() == MAX_NEIGHBORS)
			{
				break;
			}
		}

		RemoveImpossibleCornerPaths(node, NodeDetail::Small);
		CheckCollisionWithColliders(node, NodeDetail::Small, entity_manager);
	}
}

void PathFinding::CalculatePath(const PathFindData& path_find_data, std::vector<NodeIndex>& path)
{
	assert(false);
	//AStarPathfinding(path_find_data, path);
}

void PathFinding::PresentPath(const PathFindingActorComponent& path_finding_actor)
{
	if (path_finding_actor.cached_path.size() == 0)
	{
		return;
	}

	for (int i = path_finding_actor.last_path_index; i < path_finding_actor.cached_path.size() - 1; ++i)
	{
		const Vector2& pos = PathFinding::Get()->GetNodePosition(path_finding_actor.cached_path[i]);
		if (i + 1 < path_finding_actor.cached_path.size())
		{
			RenderCore::Get()->AddLine(Vector2(pos.x, pos.y));
			const Vector2& pos_next = PathFinding::Get()->GetNodePosition(path_finding_actor.cached_path[i + 1]);
			RenderCore::Get()->AddLine(Vector2(pos_next.x, pos_next.y));
		}
	}
}

void PathFinding::ThreadHandleRequests()
{
	while (true)
	{
		if (m_thread_state == ThreadState::Close)
		{
			break;
		}

		while (m_thread_state == ThreadState::Wait)
		{
		}

		m_calculate_paths_mutex.lock();
		if (!m_paths_wait_list.empty())
		{
			const std::vector<PathFindData> paths_to_calculate = m_paths_wait_list;
			m_paths_wait_list.clear();
			const std::vector<Node> world = m_nodes;

			m_calculate_paths_mutex.unlock();

			std::vector<ProcessedRequest> processed_requests;
			for (const PathFindData& path_find_data : paths_to_calculate)
			{
				CalculatedPath calculated_path{};
				calculated_path.new_path = true;

				AStarPathfinding(path_find_data, calculated_path.path, world);
				processed_requests.push_back(ProcessedRequest{ .path_find_data = path_find_data, .calculated_path = calculated_path });
			}

			m_calculate_paths_mutex.lock();
			std::ranges::copy(processed_requests, std::back_inserter(m_paths_calculated_to_be_processed));
		}
		m_calculate_paths_mutex.unlock();

		if (m_thread_state != ThreadState::Close)
		{
			m_thread_state = ThreadState::Wait;
		}
	}
}

PathFinding::Node* PathFinding::AddNeighbor(Node& node, const Vector2& neighbor_position, const NodeDetail node_detail)
{
	const auto neighbor_node_index = GetNodeFromPosition(neighbor_position, node_detail);
	if (neighbor_node_index == NULL_NODE_INDEX)
	{
		return nullptr;
	}
	Node& neighbor_node = m_nodes.at(neighbor_node_index.value);
	neighbor_node.neighbors.push_back(node.node_index);
	node.neighbors.push_back(neighbor_node_index);
	return &(neighbor_node);
}

void PathFinding::RemoveImpossibleCornerPaths(Node& node, const NodeDetail node_detail)
{
	//We want to avoid the case when a path goes between a empty place and a node
		/*
		* Example
		* A B
		* C D E
		* The most optimal path from B to E is a diagonal path, but in this case we want to avoid that the enemies get stuck on walls so we want them to go B-D-E.
		*/

	float hyp_length_between_nodes = std::sqrtf(2.0f) * LENGTH_BETWEEN_NODES;
	if (node_detail == NodeDetail::Small)
	{
		hyp_length_between_nodes = std::sqrtf(2.0f) * SMALL_LENGTH_BETWEEN_NODES;
	}

	constexpr float EPSILON = 1e-05f;
	for (size_t i{ 0 }; i < node.neighbors.size(); ++i)
	{
		const auto neighbor_index = node.neighbors[i];
		const Node& neighbor_node = m_nodes.at(neighbor_index.value);

		const Vector2 neighbor_position = neighbor_node.position;
		const Vector2 diff = node.position - neighbor_position;
		if (std::abs(diff.x) > EPSILON && std::abs(diff.y) > EPSILON)
		{
			char found_neighbors = 0;
			for (const auto compare_node_index : node.neighbors)
			{
				const Node& compare_node = m_nodes.at(compare_node_index.value);

				const Vector2 compare_node_position = compare_node.position;
				const Vector2 compare_diff = node.position - compare_node_position;
				if ((std::abs(compare_diff.x) > EPSILON && std::abs(compare_diff.y) < EPSILON) || (std::abs(compare_diff.x) < EPSILON && std::abs(compare_diff.y) > EPSILON))
				{
					const Vector2 compare_neighbor_diff = neighbor_position - compare_node_position;
					const float compare_neighbor_diff_length = compare_neighbor_diff.Length();
					if (compare_neighbor_diff_length - EPSILON > hyp_length_between_nodes)
					{
						continue;
					}
					if (std::abs(compare_neighbor_diff.x) > EPSILON)
					{
						++found_neighbors;
					}
					else if (std::abs(compare_neighbor_diff.y) > EPSILON)
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

void PathFinding::CheckCollisionWithColliders(Node& node, const NodeDetail node_detail, EntityManager* entity_manager)
{
	const float half_node_length = node_detail == NodeDetail::Base ? LENGTH_BETWEEN_NODES / 2.0f : SMALL_LENGTH_BETWEEN_NODES / 2.0f;

	entity_manager->System<TransformComponent, PureStaticBodyComponent, BoxColliderComponent>([&](const TransformComponent& transform_component, const PureStaticBodyComponent&, const BoxColliderComponent& box_collider_component)
		{
			if (box_collider_component.trigger)
			{
				return;
			}

			const Vector2 collider_position = transform_component.GetPosition2D() + box_collider_component.offset;
			const Vector2 node_position = node.position;
			const Vector2 half_box_size = box_collider_component.half_box_size;

			/*
			*	    B
			*	  *---*
			*	A |   | C
			*	  *---*
			*	    D
			*/

			const bool inside_a = node.position.x < collider_position.x + half_box_size.x;
			const bool inside_b = node.position.x + half_node_length > collider_position.x;
			const bool inside_c = node.position.y < collider_position.y + half_box_size.y;
			const bool inside_d = node.position.y + half_node_length > collider_position.y;
			if (inside_a && inside_b && inside_c && inside_d)
			{
				//node.weight = EXPENSIVE_WEIGHT;

			}
			//return;

			const bool a_inside_x = node_position.x - half_node_length > collider_position.x - half_box_size.x && node_position.x - half_node_length < collider_position.x + half_box_size.x;
			const bool c_inside_x = node_position.x + half_node_length > collider_position.x - half_box_size.x && node_position.x + half_node_length < collider_position.x + half_box_size.x;
			const bool inside_x = a_inside_x || c_inside_x;

			const bool d_inside_y = node_position.y - half_node_length > collider_position.y - half_box_size.y && node_position.y - half_node_length < collider_position.y + half_box_size.y;
			const bool b_inside_y = node_position.y + half_node_length > collider_position.y - half_box_size.y && node_position.y + half_node_length < collider_position.y + half_box_size.y;
			const bool inside_y = b_inside_y || d_inside_y;

			if (inside_x && inside_y)
			{
				node.weight = EXPENSIVE_WEIGHT;
			}
		});
}

void PathFinding::AStarPathfinding(const PathFindData& path_find_data, std::vector<NodeIndex>& path, const std::vector<Node>& world)
{
	const auto start_node_index = path_find_data.start_node_index;
	const auto goal_node_index = path_find_data.goal_node_index;

	if ((world.size() <= start_node_index.value) || world.size() <= goal_node_index.value)
	{
		return;
	}

	qr::unordered_map<NodeIndex, float, NodeIndexHasher> f_score;
	const auto cmp = [&](const Node* left, const Node* right) { return f_score.at(left->node_index) > f_score.at(right->node_index); };
	std::priority_queue<const Node*, std::vector<const Node*>, decltype(cmp)> open_set(cmp);
	qr::unordered_set<NodeIndex, NodeIndexHasher> open_set_node_indices;

	const auto& start_node = world.at(start_node_index.value);
	const auto& goal_node = world.at(goal_node_index.value);
	open_set.emplace(&(world.at(start_node_index.value)));
	qr::unordered_map<NodeIndex, NodeIndex, NodeIndexHasher> came_from;
	qr::unordered_map<NodeIndex, float, NodeIndexHasher> g_score;

	const Vector2 start_goal_direction = (goal_node.position - start_node.position).Normalize();

	g_score.insert({ start_node_index, 0.0f });
	f_score.insert({ start_node_index, Heuristic(start_goal_direction, start_node, goal_node) });

	NodeIndex previous = NodeIndex(0);
	while (open_set.size() > 0)
	{
		const auto current = open_set.top();
		open_set.pop();
		open_set_node_indices.erase(current->node_index);

		if (current->node_index == goal_node.node_index)
		{
			path.push_back(current->node_index);
			ConstructPath(came_from, current->node_index, path, world);
			std::ranges::reverse(path);
			return;
		}
		previous = current->node_index;

		for (const auto& neighbor : current->neighbors)
		{
			auto g_score_neighbor_it = g_score.find(neighbor);
			if (g_score_neighbor_it == g_score.end())
			{
				g_score.insert({neighbor, FLT_MAX});
				g_score_neighbor_it = g_score.find(neighbor);
			}
			auto& neighbor_node = world.at(neighbor.value);
			const Vector2 diff = neighbor_node.position - current->position;
			const auto tentative_g_score = g_score.at(current->node_index) + neighbor_node.weight;//1.0f;//SMALL_LENGTH_BETWEEN_NODES;//diff.Length();
			if (tentative_g_score < g_score_neighbor_it->second)
			{
				came_from.insert({ neighbor, current->node_index });
				g_score_neighbor_it->second = tentative_g_score;
				auto f_score_neighbor_it = f_score.find(neighbor);
				if (f_score_neighbor_it == f_score.end())
				{
					f_score.insert({ neighbor, FLT_MAX });
					f_score_neighbor_it = f_score.find(neighbor);
				}
				f_score_neighbor_it->second = tentative_g_score + Heuristic(start_goal_direction, neighbor_node, goal_node);

				//Neighbor not in open_set
				if (!open_set_node_indices.contains(neighbor))
				{
					open_set.push(&neighbor_node);
					open_set_node_indices.insert(neighbor);
				}
			}
		}
	}
}

uint64_t PathFinding::PositionToNodeIndex(const Vector2& position, const NodeDetail node_detail) const
{
	float length = HALF_LENGTH_BETWEEN_NODES;
	if (node_detail == NodeDetail::Small)
	{
		length = HALF_SMALL_LENGTH_BETWEEN_NODES;
	}

	const auto x = (uint32_t)(position.x / length);
	const auto y = (uint32_t)(position.y / length);

	Vector2 node_position = PositionToNodePosition(position, node_detail);
	uint64_t index = 0;
	uint8_t* ptr_un = (uint8_t*)&index;
	*((uint32_t*)(ptr_un)) = (uint32_t)(node_position.x / length);
	ptr_un += sizeof(uint32_t);
	*((uint32_t*)(ptr_un)) = (uint32_t)(node_position.y / length);

	return index;
}

PathFinding::PathFinding()
{
	s_singleton = this;
	EventCore::Get()->ListenToEvent<PathFinding::ConstructPathFindingWorldEvent>("SceneActivated", 1, PathFinding::ConstructPathFindingWorldEvent);

	m_thread_state = ThreadState::Run;
	m_calculate_paths_thread = new std::thread(&PathFinding::ThreadHandleRequests, this);
}

PathFinding::~PathFinding()
{
	m_thread_state = ThreadState::Close;
	m_calculate_paths_thread->join();
	delete m_calculate_paths_thread;
}

bool PathFinding::RequestPathFind(const SceneIndex scene_index, const Entity start_entity, const Entity goal_entity, const NodeDetail node_detail)
{
	SceneEntityData scene_entity_data{ .scene_index = scene_index, .entity = start_entity };
	if (m_entity_ongoing_pathfinds.contains(scene_entity_data))
	{
		return true;
	}

	EntityManager* entity_manager = SceneManager::GetEntityManager(scene_index);
	const TransformComponent& ts = entity_manager->GetComponent<TransformComponent>(start_entity);
	const TransformComponent& tg = entity_manager->GetComponent<TransformComponent>(goal_entity);
	const Vector2 start_position(ts.GetPosition().x, ts.GetPosition().y);
	const Vector2 goal_position(tg.GetPosition().x, tg.GetPosition().y);

	const NodeDataByNodeDetail& base_node_data = m_nodes_by_node_detail.at(GetNodeDetailIndex(node_detail));

	const auto found_start_node = base_node_data.position_to_node.find(PositionToNodeIndex(start_position, node_detail));
	const auto found_goal_node = base_node_data.position_to_node.find(PositionToNodeIndex(goal_position, node_detail));
	if (found_start_node == base_node_data.position_to_node.end() || found_goal_node == base_node_data.position_to_node.end())
	{
		return false;
	}

	NodeIndex start_node_index = found_start_node->second;
	NodeIndex goal_node_index = found_goal_node->second;
	//std::swap(start_node_index, goal_node_index);

	m_wait_list_paths_to_be_processed.push_back(PathFindData{ .scene_index = scene_index, .entity = start_entity, .start_node_index = start_node_index, .goal_node_index = goal_node_index });
	m_entity_ongoing_pathfinds.insert(scene_entity_data);
	m_calculated_paths.insert({ scene_entity_data, CalculatedPath{.path = {}, .new_path = false} });

	return true;
}

std::vector<NodeIndex> PathFinding::PathFind(const SceneIndex scene_index, const Entity start_entity, const Entity goal_entity, bool& new_path, const NodeDetail node_detail)
{
	SceneEntityData scene_entity_data{ .scene_index = scene_index, .entity = start_entity };
	std::vector<NodeIndex> path;
	if (!RequestPathFind(scene_index, start_entity, goal_entity, node_detail))
	{
		new_path = false;
		return {};
	}
	auto it = m_calculated_paths.find(scene_entity_data);
	new_path = it->second.new_path;
	it->second.new_path = false;
	return it->second.path;
}

std::vector<NodeIndex> PathFinding::GetPath(const SceneIndex scene_index, const Entity entity)
{
	SceneEntityData scene_entity_data{ .scene_index = scene_index, .entity = entity };
	auto it = m_calculated_paths.find(scene_entity_data);
	it->second.new_path = false;
	return it->second.path;
}

NodeIndex PathFinding::GetNodeFromPosition(const Vector2& position, const NodeDetail node_detail) const
{
	const NodeDataByNodeDetail& base_node_data = m_nodes_by_node_detail.at(GetNodeDetailIndex(node_detail));

	const auto it = base_node_data.position_to_node.find(PositionToNodeIndex(position, node_detail));
	if (it != base_node_data.position_to_node.end())
	{
		return it->second;
	}
	return NULL_NODE_INDEX;
}

Entity PathFinding::GetEntityFromNodeIndex(const NodeIndex node_index) const
{
	if (const auto& it = m_node_to_entity.find(node_index); it != m_node_to_entity.end())
	{
		return it->second;
	}

	return NULL_ENTITY;
}

bool PathFinding::IsWorldConstructed() const
{
	return !m_nodes.empty();
}

void PathFinding::AddNewNodeInternal(const SceneIndex scene_index, const Entity entity)
{
	const NodeDetail node_detail = NodeDetail::Base;

	EntityManager* entity_manager = SceneManager::GetEntityManager(scene_index);
	const auto& transform_component = entity_manager->GetComponent<TransformComponent>(entity);
	const auto& path_finding_world = entity_manager->GetComponent<PathFindingWorldComponent>(entity);

	const NodeIndex node_index = AddNewNodeInternal(transform_component.GetPosition2D(), path_finding_world, node_detail);

	Node& node = m_nodes.at(node_index.value);
	node.sub_nodes.push_back(AddNewNodeInternal(transform_component.GetPosition2D() + Vector2(-HALF_SMALL_LENGTH_BETWEEN_NODES, HALF_SMALL_LENGTH_BETWEEN_NODES), path_finding_world, NodeDetail::Small));
	node.sub_nodes.push_back(AddNewNodeInternal(transform_component.GetPosition2D() + Vector2(HALF_SMALL_LENGTH_BETWEEN_NODES, HALF_SMALL_LENGTH_BETWEEN_NODES), path_finding_world, NodeDetail::Small));
	node.sub_nodes.push_back(AddNewNodeInternal(transform_component.GetPosition2D() + Vector2(-HALF_SMALL_LENGTH_BETWEEN_NODES, -HALF_SMALL_LENGTH_BETWEEN_NODES), path_finding_world, NodeDetail::Small));
	node.sub_nodes.push_back(AddNewNodeInternal(transform_component.GetPosition2D() + Vector2(HALF_SMALL_LENGTH_BETWEEN_NODES, -HALF_SMALL_LENGTH_BETWEEN_NODES), path_finding_world, NodeDetail::Small));
}

NodeIndex PathFinding::AddNewNodeInternal(const Vector2& postion, const PathFindingWorldComponent& path_finding_world, const NodeDetail node_detail)
{
	const Vector2 position(postion.x, postion.y);

	float distance_between_nodes = WEIGHT_BETWEEN_NODES;
	if (node_detail == NodeDetail::Small)
	{
		distance_between_nodes = SMALL_LENGTH_BETWEEN_NODES;
	}

	const Vector2 left_position(position.x - distance_between_nodes, position.y);
	const Vector2 right_position(position.x + distance_between_nodes, position.y);
	const Vector2 up_position(position.x, position.y + distance_between_nodes);
	const Vector2 down_position(position.x, position.y - distance_between_nodes);

	const Vector2 left_up_corner(position.x - distance_between_nodes, position.y + distance_between_nodes);
	const Vector2 left_down_corner(position.x - distance_between_nodes, position.y - distance_between_nodes);
	const Vector2 right_up_corner(position.x + distance_between_nodes, position.y + distance_between_nodes);
	const Vector2 right_down_corner(position.x + distance_between_nodes, position.y - distance_between_nodes);

	NodeIndex node_index{};
	if (m_removed_node_indices.empty())
	{
		node_index = m_next_node_index;
		m_next_node_index.increase();
		m_nodes.push_back(Node(node_index, path_finding_world, position));
	}
	else
	{
		node_index = m_removed_node_indices.back();
		m_removed_node_indices.pop_back();
		m_nodes[node_index.value] = Node(node_index, path_finding_world, position);
	}

	m_nodes_by_node_detail[GetNodeDetailIndex(node_detail)].position_to_node.insert({ PositionToNodeIndex(position, node_detail), node_index });
	auto& new_node = m_nodes.at(node_index.value);

	std::vector<Node*> changed_nodes = { &(new_node) };

	changed_nodes.push_back(AddNeighbor(new_node, left_position, node_detail));
	changed_nodes.push_back(AddNeighbor(new_node, right_position, node_detail));
	changed_nodes.push_back(AddNeighbor(new_node, up_position, node_detail));
	changed_nodes.push_back(AddNeighbor(new_node, down_position, node_detail));

	changed_nodes.push_back(AddNeighbor(new_node, left_up_corner, node_detail));
	changed_nodes.push_back(AddNeighbor(new_node, left_down_corner, node_detail));
	changed_nodes.push_back(AddNeighbor(new_node, right_up_corner, node_detail));
	changed_nodes.push_back(AddNeighbor(new_node, right_down_corner, node_detail));

	for (Node* node : changed_nodes)
	{
		if (node)
		{
			RemoveImpossibleCornerPaths(*node, node_detail);
		}
	}

	return node_index;
}

void PathFinding::RemoveNodeInternal(const SceneIndex scene_index, const Entity entity)
{
	const NodeDetail node_detail = NodeDetail::Base;

	EntityManager* entity_manager = SceneManager::GetEntityManager(scene_index);
	const auto& transform_component = entity_manager->GetComponent<TransformComponent>(entity);
	const auto& path_finding_world = entity_manager->GetComponent<PathFindingWorldComponent>(entity);

	const Vector2 position(transform_component.GetPosition().x, transform_component.GetPosition().y);

	NodeDataByNodeDetail& node_by_detail = m_nodes_by_node_detail.at(GetNodeDetailIndex(node_detail));
	const auto it = node_by_detail.position_to_node.find(PositionToNodeIndex(position, node_detail));
	const NodeIndex node_index = it->second;
	const Node& node = m_nodes.at(node_index.value);
	node_by_detail.position_to_node.erase(it);

	RemoveNodeFromNeighbours(node_index, node_detail);
	for (const NodeIndex sub_node_index : node.sub_nodes)
	{
		RemoveNodeFromNeighbours(sub_node_index, node_detail);
		m_removed_node_indices.push_back(sub_node_index);
	}

	m_removed_node_indices.push_back(node_index);
}

void PathFinding::RemoveNodeFromNeighbours(const NodeIndex node_index, const NodeDetail node_detail)
{
	const Node& remove_node = m_nodes.at(node_index.value);
	for (const NodeIndex remove_node_neighbor_index : remove_node.neighbors)
	{
		Node& remove_node_neighbor_node = m_nodes.at(remove_node_neighbor_index.value);
		for (int i = 0; i < remove_node_neighbor_node.neighbors.size(); ++i)
		{
			if (remove_node_neighbor_node.neighbors[i] == node_index)
			{
				remove_node_neighbor_node.neighbors.erase(remove_node_neighbor_node.neighbors.begin() + i);
				break;
			}
		}
		RemoveImpossibleCornerPaths(remove_node_neighbor_node, node_detail);
	}
}

Vector2 PathFinding::GetNodePosition(const NodeIndex node_index) const
{
	return m_nodes.at(node_index.value).position;
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

	entity_manager->System<DeferredEntityDeletion, PathFindingActorComponent>([&](const Entity entity, DeferredEntityDeletion, const PathFindingActorComponent&)
		{
			const SceneEntityData scene_entity_data{ .scene_index = entity_manager->GetSceneIndex(), .entity = entity};
			if (m_entity_ongoing_pathfinds.contains(scene_entity_data))
			{
				m_entity_ongoing_pathfinds.erase(scene_entity_data);
			}
			if (m_calculated_paths.contains(scene_entity_data))
			{
				m_calculated_paths.erase(scene_entity_data);
			}
		});
}

void PathFinding::HandlePathFinding()
{
	SceneManager::GetEntityManager(SceneManager::GetActiveSceneIndex())->System<PathFindingActorComponent>([&](const Entity entity, const PathFindingActorComponent& path_finding_actor_component)
		{
			if (path_finding_actor_component.show_path)
			{
				PresentPath(path_finding_actor_component);
			}
		});

	m_calculate_paths_mutex.lock();

	for (const auto it : m_paths_calculated_to_be_processed)
	{
		SceneEntityData scene_entity_data{ .scene_index = it.path_find_data.scene_index, .entity = it.path_find_data.entity };
		if (!SceneManager::GetSceneManager()->SceneExists(scene_entity_data.scene_index))
		{
			continue;
		}
		if (!SceneManager::GetEntityManager(scene_entity_data.scene_index)->EntityExists(scene_entity_data.entity))
		{
			continue;
		}

		m_calculated_paths.at(scene_entity_data) = it.calculated_path;
		m_calculated_paths.at(scene_entity_data).new_path = false;
		m_entity_ongoing_pathfinds.erase(scene_entity_data);

		EventCore::Get()->SendEvent("NewPathFinding", it.calculated_path.path, scene_entity_data.scene_index, scene_entity_data.entity);
	}
	m_paths_calculated_to_be_processed.clear();

	for (const auto& path_to_calculcate : m_wait_list_paths_to_be_processed)
	{
		m_paths_wait_list.push_back(path_to_calculcate);
	}
	m_wait_list_paths_to_be_processed.clear();

	for (const SceneEntityData& scene_entity_data : m_nodes_to_be_added)
	{
		AddNewNodeInternal(scene_entity_data.scene_index, scene_entity_data.entity);
	}
	m_nodes_to_be_added.clear();

	for (const SceneEntityData& scene_entity_data : m_nodes_to_be_removed)
	{
		RemoveNodeInternal(scene_entity_data.scene_index, scene_entity_data.entity);
	}
	m_nodes_to_be_removed.clear();

	if (!m_paths_wait_list.empty())
	{
		m_thread_state = ThreadState::Run;
	}

	m_calculate_paths_mutex.unlock();
}

void PathFinding::AddNewNode(const SceneIndex scene_index, const Entity entity)
{
	m_nodes_to_be_added.push_back(SceneEntityData{.scene_index = scene_index, .entity = entity});
}

void PathFinding::RemoveNode(const SceneIndex scene_index, const Entity entity)
{
	m_nodes_to_be_removed.push_back(SceneEntityData{ .scene_index = scene_index, .entity = entity });
}

PathFinding* PathFinding::Get()
{
	return s_singleton;
}
