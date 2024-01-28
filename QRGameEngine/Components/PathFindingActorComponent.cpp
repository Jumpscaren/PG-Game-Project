#include "pch.h"
#include "PathFindingActorComponent.h"
#include "Scripting/CSMonoCore.h"
#include "SceneSystem/SceneLoader.h"
#include "SceneSystem/SceneManager.h"

void PathFindingActorComponentInterface::RegisterInterface(CSMonoCore* mono_core)
{
	const auto path_finding_actor_class = mono_core->RegisterMonoClass("ScriptProject.Engine", "PathFindingActor");

	mono_core->HookAndRegisterMonoMethodType<PathFindingActorComponentInterface::InitComponent>(path_finding_actor_class, "InitComponent", PathFindingActorComponentInterface::InitComponent);
	mono_core->HookAndRegisterMonoMethodType<PathFindingActorComponentInterface::HasComponent>(path_finding_actor_class, "HasComponent", PathFindingActorComponentInterface::HasComponent);
	mono_core->HookAndRegisterMonoMethodType<PathFindingActorComponentInterface::RemoveComponent>(path_finding_actor_class, "RemoveComponent", PathFindingActorComponentInterface::RemoveComponent);

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

void PathFindingActorComponentInterface::SavePathFindingWorldComponent(const Entity ent, EntityManager* entman, JsonObject* json_object)
{
}

void PathFindingActorComponentInterface::LoadPathFindingWorldComponent(const Entity ent, EntityManager* entman, JsonObject* json_object)
{
}