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
#include "Renderer/RenderCore.h"
#include "Scripting/Objects/ListSetInterface.h"
#include "Event/EventCore.h"

void PathFindingActorComponentInterface::RegisterInterface(CSMonoCore* mono_core)
{
	const auto path_finding_actor_class = mono_core->RegisterMonoClass("ScriptProject.Engine", "PathFindingActor");

	mono_core->HookAndRegisterMonoMethodType<PathFindingActorComponentInterface::InitComponent>(path_finding_actor_class, "InitComponent", PathFindingActorComponentInterface::InitComponent);
	mono_core->HookAndRegisterMonoMethodType<PathFindingActorComponentInterface::HasComponent>(path_finding_actor_class, "HasComponent", PathFindingActorComponentInterface::HasComponent);
	mono_core->HookAndRegisterMonoMethodType<PathFindingActorComponentInterface::RemoveComponent>(path_finding_actor_class, "RemoveComponent", PathFindingActorComponentInterface::RemoveComponent);

	mono_core->HookAndRegisterMonoMethodType<PathFindingActorComponentInterface::PathFind>(path_finding_actor_class, "PathFind_Extern", PathFindingActorComponentInterface::PathFind);
	mono_core->HookAndRegisterMonoMethodType<PathFindingActorComponentInterface::GetCurrentNodePosition>(path_finding_actor_class, "GetCurrentNodePosition_Extern", PathFindingActorComponentInterface::GetCurrentNodePosition);
	mono_core->HookAndRegisterMonoMethodType<PathFindingActorComponentInterface::GetNextNodePosition>(path_finding_actor_class, "GetNextNodePosition_Extern", PathFindingActorComponentInterface::GetNextNodePosition);
	mono_core->HookAndRegisterMonoMethodType<PathFindingActorComponentInterface::DebugPath>(path_finding_actor_class, "DebugPath", PathFindingActorComponentInterface::DebugPath);
	mono_core->HookAndRegisterMonoMethodType<PathFindingActorComponentInterface::GetGameObjectNodeByPosition>(path_finding_actor_class, "GetGameObjectNodeByPosition", PathFindingActorComponentInterface::GetGameObjectNodeByPosition);
	mono_core->HookAndRegisterMonoMethodType<PathFindingActorComponentInterface::IsPositionInWorld>(path_finding_actor_class, "IsPositionInWorld", PathFindingActorComponentInterface::IsPositionInWorld);
	mono_core->HookAndRegisterMonoMethodType<PathFindingActorComponentInterface::GetRandomNodes>(path_finding_actor_class, "GetRandomNodes", PathFindingActorComponentInterface::GetRandomNodes);

	SceneLoader::Get()->OverrideSaveComponentMethod<PathFindingActorComponent>(SavePathFindingWorldComponent, LoadPathFindingWorldComponent);

	EventCore::Get()->ListenToEvent<PathFindingActorComponentInterface::ReceiveNewPath>("NewPathFinding", 0, PathFindingActorComponentInterface::ReceiveNewPath);
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

void PathFindingActorComponentInterface::PathFind(const SceneIndex actor_scene_index, const Entity actor_entity, const SceneIndex goal_scene_index, const Entity goal_entity, const uint32_t position_of_node_index)
{
	{
		const Entity entity = actor_entity;
		const SceneIndex scene_index = actor_scene_index;
		const TransformComponent& transform = SceneManager::GetEntityManager(scene_index)->GetComponent<TransformComponent>(entity);

		PathFindingActorComponent& path_finding_actor = SceneManager::GetEntityManager(scene_index)->GetComponent<PathFindingActorComponent>(entity);

		bool new_path;
		std::vector<NodeIndex> path = PathFinding::Get()->PathFind(scene_index, entity, goal_entity, new_path, NodeDetail::Small);
		if (new_path)
		{
			Vector2 actor_position_difference = PathFinding::Get()->GetNodePosition(path_finding_actor.cached_path.at(path_finding_actor.last_path_index)) - PathFinding::Get()->GetNodePosition(path.front());
			Vector2 target_position_difference = PathFinding::Get()->GetNodePosition(path_finding_actor.cached_path.back()) - PathFinding::Get()->GetNodePosition(path.back());

			if (actor_position_difference.Length() < 1.0f && target_position_difference.Length() < 1.0f)
			{
				return;
			}

			path_finding_actor.cached_path = path;
			path_finding_actor.last_path_index = 0;
		}
		else
		{
			path_finding_actor.cached_path.clear();
		}

		return;
	}
}

CSMonoObject PathFindingActorComponentInterface::GetCurrentNodePosition(const SceneIndex actor_scene_index, const Entity actor_entity)
{
	const Entity entity = actor_entity;
	const SceneIndex scene_index = actor_scene_index;

	PathFindingActorComponent& path_finding_actor = SceneManager::GetEntityManager(scene_index)->GetComponent<PathFindingActorComponent>(entity);

	if (path_finding_actor.cached_path.empty())
	{
		const TransformComponent& transform = SceneManager::GetEntityManager(scene_index)->GetComponent<TransformComponent>(entity);
		return Vector2Interface::CreateVector2(transform.GetPosition2D());
	}

	return Vector2Interface::CreateVector2(PathFinding::Get()->GetNodePosition(path_finding_actor.cached_path[path_finding_actor.last_path_index]));
}

CSMonoObject PathFindingActorComponentInterface::GetNextNodePosition(const SceneIndex actor_scene_index, const Entity actor_entity, const uint32_t position_of_node_index)
{
	const Entity entity = actor_entity;
	const SceneIndex scene_index = actor_scene_index;

	PathFindingActorComponent& path_finding_actor = SceneManager::GetEntityManager(scene_index)->GetComponent<PathFindingActorComponent>(entity);

	if (path_finding_actor.cached_path.empty())
	{
		const TransformComponent& transform = SceneManager::GetEntityManager(scene_index)->GetComponent<TransformComponent>(entity);
		return Vector2Interface::CreateVector2(transform.GetPosition2D());
	}

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
	if (next_path_node_index < path_finding_actor.cached_path.size())
	{
		path_finding_actor.last_path_index = next_path_node_index;
	}

	return Vector2Interface::CreateVector2(PathFinding::Get()->GetNodePosition(path_finding_actor.cached_path[next_path_node_index]));
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
		const Vector2& pos = PathFinding::Get()->GetNodePosition(path_finding_actor.cached_path[i]);
		if (i + 1 < path_finding_actor.cached_path.size())
		{
			RenderCore::Get()->AddLine(Vector2(pos.x, pos.y));
			const Vector2& pos_next = PathFinding::Get()->GetNodePosition(path_finding_actor.cached_path[i + 1]);
			RenderCore::Get()->AddLine(Vector2(pos_next.x, pos_next.y));
		}
	}
}

CSMonoObject PathFindingActorComponentInterface::GetGameObjectNodeByPosition(const CSMonoObject& position)
{
	const Vector2 position_vector = Vector2Interface::GetVector2(position);
	const NodeIndex node = PathFinding::Get()->GetNodeFromPosition(position_vector, NodeDetail::Base);
	const Entity entity = PathFinding::Get()->GetEntityFromNodeIndex(node);
	return GameObjectInterface::NewGameObjectWithExistingEntity(entity, SceneManager::GetActiveSceneIndex());
}

bool PathFindingActorComponentInterface::IsPositionInWorld(const CSMonoObject& position)
{
	const Vector2 position_vector = Vector2Interface::GetVector2(position);
	const NodeIndex node = PathFinding::Get()->GetNodeFromPosition(position_vector, NodeDetail::Base);
	return node != NULL_NODE_INDEX;
}

// TODO: Move to PG-Game-Project
void PathFindingActorComponentInterface::GetRandomNodes(const CSMonoObject& game_object, const CSMonoObject& list, const uint32_t number_of_nodes)
{
	const Entity entity = GameObjectInterface::GetEntityID(game_object);
	const SceneIndex scene_index = GameObjectInterface::GetSceneIndex(game_object);
	EntityManager* entity_manager = SceneManager::GetEntityManager(scene_index);

	const std::vector<Entity>& entities = entity_manager->GetEntityList<TransformComponent, PathFindingWorldComponent>();

	for (uint32_t i = 0; i < number_of_nodes; ++i)
	{
		const auto index = rand() % entities.size();
		ListSetInterface::AddGameObject(list, GameObjectInterface::NewGameObjectWithExistingEntity(entities[index], scene_index));
	}
}

void PathFindingActorComponentInterface::ReceiveNewPath(const std::vector<NodeIndex>& path, const SceneIndex actor_scene_index, const Entity actor_entity)
{
	PathFindingActorComponent& path_finding_actor = SceneManager::GetEntityManager(actor_scene_index)->GetComponent<PathFindingActorComponent>(actor_entity);

	if (!path_finding_actor.cached_path.empty())
	{
		Vector2 actor_position_difference = PathFinding::Get()->GetNodePosition(path_finding_actor.cached_path.at(path_finding_actor.last_path_index)) - PathFinding::Get()->GetNodePosition(path.front());
		Vector2 target_position_difference = PathFinding::Get()->GetNodePosition(path_finding_actor.cached_path.back()) - PathFinding::Get()->GetNodePosition(path.back());

		if (actor_position_difference.Length() < 1.0f && target_position_difference.Length() < 1.0f)
		{
			return;
		}
	}

	path_finding_actor.cached_path = path;
	path_finding_actor.last_path_index = 1;
}

void PathFindingActorComponentInterface::SavePathFindingWorldComponent(const Entity ent, EntityManager* entman, JsonObject* json_object)
{
}

void PathFindingActorComponentInterface::LoadPathFindingWorldComponent(const Entity ent, EntityManager* entman, JsonObject* json_object)
{
}