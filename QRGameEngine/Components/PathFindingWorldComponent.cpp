#include "pch.h"
#include "PathFindingWorldComponent.h"
#include "Scripting/CSMonoCore.h"
#include "SceneSystem/SceneLoader.h"
#include "SceneSystem/SceneManager.h"

void PathFindingWorldComponentInterface::RegisterInterface(CSMonoCore* mono_core)
{
	const auto path_finding_world_class = mono_core->RegisterMonoClass("ScriptProject.Engine", "PathFindingWorld");

	mono_core->HookAndRegisterMonoMethodType<PathFindingWorldComponentInterface::InitComponent>(path_finding_world_class, "InitComponent", PathFindingWorldComponentInterface::InitComponent);
	mono_core->HookAndRegisterMonoMethodType<PathFindingWorldComponentInterface::HasComponent>(path_finding_world_class, "HasComponent", PathFindingWorldComponentInterface::HasComponent);
	mono_core->HookAndRegisterMonoMethodType<PathFindingWorldComponentInterface::RemoveComponent>(path_finding_world_class, "RemoveComponent", PathFindingWorldComponentInterface::RemoveComponent);

	SceneLoader::Get()->OverrideSaveComponentMethod<PathFindingWorldComponent>(SavePathFindingWorldComponent, LoadPathFindingWorldComponent);
}

void PathFindingWorldComponentInterface::InitComponent(const CSMonoObject, const SceneIndex scene_index, const Entity entity)
{
	SceneManager::GetEntityManager(scene_index)->AddComponent<PathFindingWorldComponent>(entity);
}

bool PathFindingWorldComponentInterface::HasComponent(const CSMonoObject object, const SceneIndex scene_index, const Entity entity)
{
	return SceneManager::GetEntityManager(scene_index)->HasComponent<PathFindingWorldComponent>(entity);
}

void PathFindingWorldComponentInterface::RemoveComponent(const CSMonoObject object, const SceneIndex scene_index, const Entity entity)
{
	SceneManager::GetEntityManager(scene_index)->RemoveComponent<PathFindingWorldComponent>(entity);
}

void PathFindingWorldComponentInterface::SavePathFindingWorldComponent(const Entity ent, EntityManager* entman, JsonObject* json_object)
{
}

void PathFindingWorldComponentInterface::LoadPathFindingWorldComponent(const Entity ent, EntityManager* entman, JsonObject* json_object)
{
}
