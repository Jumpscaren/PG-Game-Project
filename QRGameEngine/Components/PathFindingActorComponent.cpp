#include "pch.h"
#include "PathFindingActorComponent.h"
#include "Scripting/CSMonoCore.h"
#include "SceneSystem/SceneLoader.h"
#include "SceneSystem/SceneManager.h"
#include "ComponentInterface.h"
#include "Scripting/Objects/GameObjectInterface.h"
#include "Scripting/Objects/Vector2Interface.h"
#include "PathFinding.h"
#include "TransformComponent.h"
#include "SceneSystem/SceneManager.h"
#include "Renderer/RenderCore.h"

void PathFindingActorComponentInterface::RegisterInterface(CSMonoCore* mono_core)
{
	const auto path_finding_actor_class = mono_core->RegisterMonoClass("ScriptProject.Engine", "PathFindingActor");

	mono_core->HookAndRegisterMonoMethodType<PathFindingActorComponentInterface::InitComponent>(path_finding_actor_class, "InitComponent", PathFindingActorComponentInterface::InitComponent);
	mono_core->HookAndRegisterMonoMethodType<PathFindingActorComponentInterface::HasComponent>(path_finding_actor_class, "HasComponent", PathFindingActorComponentInterface::HasComponent);
	mono_core->HookAndRegisterMonoMethodType<PathFindingActorComponentInterface::RemoveComponent>(path_finding_actor_class, "RemoveComponent", PathFindingActorComponentInterface::RemoveComponent);

	mono_core->HookAndRegisterMonoMethodType<PathFindingActorComponentInterface::PathFind>(path_finding_actor_class, "PathFind", PathFindingActorComponentInterface::PathFind);

	SceneLoader::Get()->OverrideSaveComponentMethod<PathFindingActorComponent>(SavePathFindingWorldComponent, LoadPathFindingWorldComponent);
}

void PathFindingActorComponentInterface::InitComponent(const CSMonoObject, const SceneIndex scene_index, const Entity entity)
{
	SceneManager::GetEntityManager(scene_index)->AddComponent<PathFindingActorComponent>(entity);
}

bool PathFindingActorComponentInterface::HasComponent(const CSMonoObject object, const SceneIndex scene_index, const Entity entity)
{
	return SceneManager::GetEntityManager(scene_index)->HasComponent<PathFindingActorComponent>(entity);
}

void PathFindingActorComponentInterface::RemoveComponent(const CSMonoObject object, const SceneIndex scene_index, const Entity entity)
{
	SceneManager::GetEntityManager(scene_index)->RemoveComponent<PathFindingActorComponent>(entity);
}

CSMonoObject PathFindingActorComponentInterface::PathFind(const CSMonoObject object, const CSMonoObject goal_game_object, const uint32_t position_of_node_index)
{
	const CSMonoObject& game_object = ComponentInterface::GetGameObject(object);
	const Entity entity = GameObjectInterface::GetEntityID(game_object);
	const SceneIndex scene_index = GameObjectInterface::GetSceneIndex(game_object);
	const Entity goal_entity = GameObjectInterface::GetEntityID(goal_game_object);

	PathFindingActorComponent& path_finding_actor = SceneManager::GetEntityManager(scene_index)->GetComponent<PathFindingActorComponent>(entity);

	const TransformComponent& transform = SceneManager::GetEntityManager(scene_index)->GetComponent<TransformComponent>(entity);
	const Vector2 current_node_position(transform.GetPosition().x, transform.GetPosition().y);
	const auto current_node = PathFinding::Get()->GetNodeFromPosition(current_node_position);
	auto next_path_node_index = path_finding_actor.last_path_index + position_of_node_index;
	if (next_path_node_index >= path_finding_actor.cached_path.size())
	{
		next_path_node_index = (uint32_t)(path_finding_actor.cached_path.size() - 1);
	}
	if (next_path_node_index < path_finding_actor.cached_path.size() && current_node == path_finding_actor.cached_path[next_path_node_index])
	{
		path_finding_actor.last_path_index = next_path_node_index;
	}
	if (current_node != NULL_ENTITY)
	{
		path_finding_actor.last_visited_node = current_node;
	}

	Vector2 node_position;
	const auto& path_position_3 = SceneManager::GetEntityManager(scene_index)->GetComponent<TransformComponent>(path_finding_actor.goal_last_visited_node).GetPosition();
	node_position.x = path_position_3.x;
	node_position.y = path_position_3.y;
	if (next_path_node_index < path_finding_actor.cached_path.size())
	{
		const auto& path_position_3 = SceneManager::GetEntityManager(scene_index)->GetComponent<TransformComponent>(path_finding_actor.cached_path[next_path_node_index]).GetPosition();
		node_position.x = path_position_3.x;
		node_position.y = path_position_3.y;
	}
	const TransformComponent& goal_transform = SceneManager::GetEntityManager(scene_index)->GetComponent<TransformComponent>(goal_entity);
	const Entity goal_node = PathFinding::Get()->GetNodeFromPosition(Vector2(goal_transform.GetPosition().x, goal_transform.GetPosition().y));

	const auto path_find = goal_node != path_finding_actor.goal_last_visited_node || (current_node_position - node_position).Length() > 2.0f;
	if (path_find && goal_node != NULL_ENTITY)
	{
		path_finding_actor.cached_path = PathFinding::Get()->PathFind(scene_index, entity, goal_entity);
		path_finding_actor.goal_last_visited_node = goal_node;
		path_finding_actor.last_path_index = 0;
	}

	for (int i = next_path_node_index > 0 ? next_path_node_index - 1 : 0; i < path_finding_actor.cached_path.size(); ++i)
	{
		const TransformComponent& trans = SceneManager::GetEntityManager(scene_index)->GetComponent<TransformComponent>(path_finding_actor.cached_path[i]);
		if (i + 1 < path_finding_actor.cached_path.size())
		{
			RenderCore::Get()->AddLine(Vector2(trans.GetPosition().x, trans.GetPosition().y));
			const TransformComponent& trans_next = SceneManager::GetEntityManager(scene_index)->GetComponent<TransformComponent>(path_finding_actor.cached_path[i + 1]);
			RenderCore::Get()->AddLine(Vector2(trans_next.GetPosition().x, trans_next.GetPosition().y));
		}
	}

	return Vector2Interface::CreateVector2(node_position);
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