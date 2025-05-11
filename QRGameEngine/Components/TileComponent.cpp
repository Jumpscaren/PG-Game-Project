#include "pch.h"
#include "TileComponent.h"
#include "SceneSystem/SceneLoader.h"
#include "Scripting/CSMonoCore.h"
#include "SceneSystem/SceneManager.h"

void TileComponentInterface::RegisterInterface(CSMonoCore* mono_core)
{
	const auto static_body_class = mono_core->RegisterMonoClass("ScriptProject.Engine", "Tile");

	mono_core->HookAndRegisterMonoMethodType<TileComponentInterface::InitComponent>(static_body_class, "InitComponent", TileComponentInterface::InitComponent);
	mono_core->HookAndRegisterMonoMethodType<TileComponentInterface::HasComponent>(static_body_class, "HasComponent", TileComponentInterface::HasComponent);
	mono_core->HookAndRegisterMonoMethodType<TileComponentInterface::RemoveComponent>(static_body_class, "RemoveComponent", TileComponentInterface::RemoveComponent);

	SceneLoader::Get()->OverrideSaveComponentMethod<TileComponent>(SaveScriptComponent, LoadScriptComponent);
}

void TileComponentInterface::InitComponent(const CSMonoObject& object, SceneIndex scene_index, Entity entity)
{
	SceneManager::GetEntityManager(scene_index)->AddComponent<TileComponent>(entity);
}

bool TileComponentInterface::HasComponent(const CSMonoObject& object, SceneIndex scene_index, Entity entity)
{
	return SceneManager::GetEntityManager(scene_index)->HasComponent<TileComponent>(entity);
}

void TileComponentInterface::RemoveComponent(const CSMonoObject& object, SceneIndex scene_index, Entity entity)
{
	SceneManager::GetEntityManager(scene_index)->RemoveComponent<TileComponent>(entity);
}

void TileComponentInterface::SaveScriptComponent(Entity ent, EntityManager* entman, JsonObject* json_object)
{
}

void TileComponentInterface::LoadScriptComponent(Entity ent, EntityManager* entman, JsonObject* json_object)
{
}
