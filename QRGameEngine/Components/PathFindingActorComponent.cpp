#include "pch.h"
#include "PathFindingActorComponent.h"
#include "Scripting/CSMonoCore.h"
#include "SceneSystem/SceneLoader.h"
#include "SceneSystem/SceneManager.h"
#include "ComponentInterface.h"
#include "Scripting/Objects/GameObjectInterface.h"
#include "Scripting/Objects/Vector2Interface.h"
#include "PathFinding/PathFinding.h"
#include "TransformComponent.h"
#include "SceneSystem/SceneManager.h"
#include "Renderer/RenderCore.h"

void PathFindingActorComponentInterface::RegisterInterface(CSMonoCore* mono_core)
{
	const auto path_finding_actor_class = mono_core->RegisterMonoClass("ScriptProject.Engine", "PathFindingActor");

	mono_core->HookAndRegisterMonoMethodType<PathFindingActorComponentInterface::InitComponent>(path_finding_actor_class, "InitComponent", PathFindingActorComponentInterface::InitComponent);
	mono_core->HookAndRegisterMonoMethodType<PathFindingActorComponentInterface::HasComponent>(path_finding_actor_class, "HasComponent", PathFindingActorComponentInterface::HasComponent);
	mono_core->HookAndRegisterMonoMethodType<PathFindingActorComponentInterface::RemoveComponent>(path_finding_actor_class, "RemoveComponent", PathFindingActorComponentInterface::RemoveComponent);

	mono_core->HookAndRegisterMonoMethodType<PathFindingActorComponentInterface::PathFind>(path_finding_actor_class, "PathFind_Extern", PathFindingActorComponentInterface::PathFind);
	mono_core->HookAndRegisterMonoMethodType<PathFindingActorComponentInterface::DebugPath>(path_finding_actor_class, "DebugPath", PathFindingActorComponentInterface::DebugPath);
	mono_core->HookAndRegisterMonoMethodType<PathFindingActorComponentInterface::NeedNewPathFind>(path_finding_actor_class, "NeedNewPathFind_Extern", PathFindingActorComponentInterface::NeedNewPathFind);

	SceneLoader::Get()->OverrideSaveComponentMethod<PathFindingActorComponent>(SavePathFindingWorldComponent, LoadPathFindingWorldComponent);
}

void PathFindingActorComponentInterface::InitComponent(const CSMonoObject&, const SceneIndex scene_index, const Entity entity)
{
	SceneManager::GetEntityManager(scene_index)->AddComponent<PathFindingActorComponent>(entity);
}

bool PathFindingActorComponentInterface::HasComponent(const CSMonoObject& object, const SceneIndex scene_index, const Entity entity)
{
	return SceneManager::GetEntityManager(scene_index)->HasComponent<PathFindingActorComponent>(entity);
}

void PathFindingActorComponentInterface::RemoveComponent(const CSMonoObject& object, const SceneIndex scene_index, const Entity entity)
{
	SceneManager::GetEntityManager(scene_index)->RemoveComponent<PathFindingActorComponent>(entity);
}

CSMonoObject PathFindingActorComponentInterface::PathFind(const SceneIndex actor_scene_index, const Entity actor_entity, const SceneIndex goal_scene_index, const Entity goal_entity, const uint32_t position_of_node_index)
{
	const Entity entity = actor_entity;
	const SceneIndex scene_index = actor_scene_index;

	PathFindingActorComponent& path_finding_actor = SceneManager::GetEntityManager(scene_index)->GetComponent<PathFindingActorComponent>(entity);

	const TransformComponent& transform = SceneManager::GetEntityManager(scene_index)->GetComponent<TransformComponent>(entity);
	const Vector2 current_node_position(transform.GetPosition().x, transform.GetPosition().y);
	const auto current_node = PathFinding::Get()->GetNodeFromPosition(current_node_position);
	auto next_path_node_index = path_finding_actor.last_path_index + position_of_node_index;
	if (next_path_node_index >= path_finding_actor.cached_path.size())
	{
		if (path_finding_actor.cached_path.size() > 0)
		{
			next_path_node_index = (uint32_t)(path_finding_actor.cached_path.size() - 1);
		}
		else
		{
			next_path_node_index = 0;
		}
	}
	if (next_path_node_index < path_finding_actor.cached_path.size() && current_node == path_finding_actor.cached_path[next_path_node_index])
	{
		path_finding_actor.last_path_index = next_path_node_index;
	}
	if (current_node != NULL_ENTITY)
	{
		path_finding_actor.last_visited_node = current_node;
	}

	Vector2 node_position = current_node_position;
	const TransformComponent& goal_transform = SceneManager::GetEntityManager(scene_index)->GetComponent<TransformComponent>(goal_entity);
	const Entity goal_node = PathFinding::Get()->GetNodeFromPosition(Vector2(goal_transform.GetPosition().x, goal_transform.GetPosition().y));

	auto has_to_path_find = HasToPathFind(path_finding_actor, current_node, goal_node);
	if (has_to_path_find)
	{
		bool new_path;
		std::vector<Entity> path = PathFinding::Get()->PathFind(scene_index, entity, goal_entity, new_path);
		if (new_path)
		{
			path_finding_actor.cached_path = path;
			path_finding_actor.goal_last_visited_node = goal_node;
			path_finding_actor.last_path_index = 0;

			path_finding_actor.cached_mapped_path.clear();
			uint32_t path_index = 0;
			for (const auto node_entity : path_finding_actor.cached_path)
			{
				path_finding_actor.cached_mapped_path.insert(node_entity);
				if (current_node == node_entity)
				{
					path_finding_actor.last_path_index = path_index;
				}
				++path_index;
			}
		}
		else
		{
			has_to_path_find = false;
		}
	}
	if (!has_to_path_find) {
		if (path_finding_actor.goal_last_visited_node != NULL_ENTITY)
		{
			const auto& path_position_3 = SceneManager::GetEntityManager(scene_index)->GetComponent<TransformComponent>(path_finding_actor.goal_last_visited_node).GetPosition();
			node_position.x = path_position_3.x;
			node_position.y = path_position_3.y;
		}
		if (next_path_node_index < path_finding_actor.cached_path.size())
		{
			const auto& path_position_3 = SceneManager::GetEntityManager(scene_index)->GetComponent<TransformComponent>(path_finding_actor.cached_path[next_path_node_index]).GetPosition();
			node_position.x = path_position_3.x;
			node_position.y = path_position_3.y;
		}
	}

	//for (int i = next_path_node_index > 0 ? next_path_node_index - 1 : 0; i < path_finding_actor.cached_path.size(); ++i)
	//{
	//	const TransformComponent& trans = SceneManager::GetEntityManager(scene_index)->GetComponent<TransformComponent>(path_finding_actor.cached_path[i]);
	//	if (i + 1 < path_finding_actor.cached_path.size())
	//	{
	//		RenderCore::Get()->AddLine(Vector2(trans.GetPosition().x, trans.GetPosition().y));
	//		const TransformComponent& trans_next = SceneManager::GetEntityManager(scene_index)->GetComponent<TransformComponent>(path_finding_actor.cached_path[i + 1]);
	//		RenderCore::Get()->AddLine(Vector2(trans_next.GetPosition().x, trans_next.GetPosition().y));
	//	}
	//}

	return Vector2Interface::CreateVector2(node_position);
}

void PathFindingActorComponentInterface::DebugPath(const CSMonoObject& object)
{
	const CSMonoObject game_object = ComponentInterface::GetGameObject(object);
	const Entity entity = GameObjectInterface::GetEntityID(game_object);
	const SceneIndex scene_index = GameObjectInterface::GetSceneIndex(game_object);
	const PathFindingActorComponent& path_finding_actor = SceneManager::GetEntityManager(scene_index)->GetComponent<PathFindingActorComponent>(entity);

	if (path_finding_actor.cached_path.size() == 0)
	{
		return;
	}

	for (int i = path_finding_actor.last_path_index; i < path_finding_actor.cached_path.size() - 1; ++i)
	{
		const TransformComponent& trans = SceneManager::GetEntityManager(scene_index)->GetComponent<TransformComponent>(path_finding_actor.cached_path[i]);
		if (i + 1 < path_finding_actor.cached_path.size())
		{
			RenderCore::Get()->AddLine(Vector2(trans.GetPosition().x, trans.GetPosition().y));
			const TransformComponent& trans_next = SceneManager::GetEntityManager(scene_index)->GetComponent<TransformComponent>(path_finding_actor.cached_path[i + 1]);
			RenderCore::Get()->AddLine(Vector2(trans_next.GetPosition().x, trans_next.GetPosition().y));
		}
	}
}

bool PathFindingActorComponentInterface::NeedNewPathFind(const SceneIndex actor_scene_index, const Entity actor_entity, const SceneIndex goal_scene_index, const Entity goal_entity, const uint32_t position_of_node_index)
{
	const Entity entity = actor_entity;
	const SceneIndex scene_index = actor_scene_index;

	const TransformComponent& transform = SceneManager::GetEntityManager(scene_index)->GetComponent<TransformComponent>(entity);
	const Vector2 current_node_position(transform.GetPosition().x, transform.GetPosition().y);

	Vector2 node_position = current_node_position;
	const TransformComponent& goal_transform = SceneManager::GetEntityManager(scene_index)->GetComponent<TransformComponent>(goal_entity);
	const Entity goal_node = PathFinding::Get()->GetNodeFromPosition(Vector2(goal_transform.GetPosition().x, goal_transform.GetPosition().y));

	const PathFindingActorComponent& path_finding_actor = SceneManager::GetEntityManager(scene_index)->GetComponent<PathFindingActorComponent>(entity);

	if (path_finding_actor.cached_path.size() == 0)
	{
		return true;
	}

	auto next_path_node_index = path_finding_actor.last_path_index + position_of_node_index;
	if (next_path_node_index >= path_finding_actor.cached_path.size())
	{
		next_path_node_index = (uint32_t)(path_finding_actor.cached_path.size() - 1);
	}
	const auto current_node = PathFinding::Get()->GetNodeFromPosition(current_node_position);
	if (path_finding_actor.last_path_index < path_finding_actor.cached_path.size() && path_finding_actor.cached_path[path_finding_actor.last_path_index] != current_node)
	{
		return true;
	}

	return HasToPathFind(path_finding_actor, current_node, goal_node);
}

bool PathFindingActorComponentInterface::HasToPathFind(const PathFindingActorComponent& path_finding_actor, const Entity own_node, const Entity goal_node)
{
	const bool goal_node_differ = goal_node != path_finding_actor.goal_last_visited_node;
	const bool is_in_cached_path = path_finding_actor.cached_mapped_path.contains(own_node) && path_finding_actor.cached_mapped_path.contains(path_finding_actor.last_visited_node) && path_finding_actor.cached_mapped_path.contains(goal_node);
	const bool goal_is_not_null = goal_node != NULL_ENTITY;

	return (goal_node_differ || !is_in_cached_path) && goal_is_not_null && own_node != NULL_ENTITY;
}

//path_finding_actor.cached_path = PathFinding::Get()->PathFind(scene_index, entity, goal_entity);
//for (int i = 0; i < path_finding_actor.cached_path.size(); ++i)
//{
//	const TransformComponent& trans = SceneManager::GetEntityManager(scene_index)->GetComponent<TransformComponent>(path_finding_actor.cached_path[i]);
//	if (i + 1 < path_finding_actor.cached_path.size())
//	{
//		RenderCore::Get()->AddLine(Vector2(trans.GetPosition().x, trans.GetPosition().y));
//		const TransformComponent& trans_next = SceneManager::GetEntityManager(scene_index)->GetComponent<TransformComponent>(path_finding_actor.cached_path[i + 1]);
//		RenderCore::Get()->AddLine(Vector2(trans_next.GetPosition().x, trans_next.GetPosition().y));
//	}
//}
//
//Vector2 node_position;
//if (position_of_node_index < path_finding_actor.cached_path.size())
//{
//	const auto& path_position_3 = SceneManager::GetEntityManager(scene_index)->GetComponent<TransformComponent>(path_finding_actor.cached_path[position_of_node_index]).GetPosition();
//	node_position.x = path_position_3.x;
//	node_position.y = path_position_3.y;
//}

void PathFindingActorComponentInterface::SavePathFindingWorldComponent(const Entity ent, EntityManager* entman, JsonObject* json_object)
{
}

void PathFindingActorComponentInterface::LoadPathFindingWorldComponent(const Entity ent, EntityManager* entman, JsonObject* json_object)
{
}