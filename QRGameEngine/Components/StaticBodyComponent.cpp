#include "pch.h"
#include "StaticBodyComponent.h"
#include "Scripting/CSMonoCore.h"
#include "SceneSystem/SceneManager.h"
#include "ECS/EntityManager.h"
#include "Physics/PhysicsCore.h"
#include "SceneSystem/SceneLoader.h"
#include "ComponentInterface.h"
#include "Scripting/Objects/GameObjectInterface.h"

DeferedMethodIndex StaticBodyComponentInterface::s_add_physic_object_index;
DeferedMethodIndex StaticBodyComponentInterface::s_remove_physic_object_index;

void StaticBodyComponentInterface::RegisterInterface(CSMonoCore* mono_core, const DeferedMethodIndex add_physic_object_index, const DeferedMethodIndex remove_physic_object_index)
{
	const auto static_body_class = mono_core->RegisterMonoClass("ScriptProject.Engine", "StaticBody");

	mono_core->HookAndRegisterMonoMethodType<StaticBodyComponentInterface::InitComponent>(static_body_class, "InitComponent", StaticBodyComponentInterface::InitComponent);
	mono_core->HookAndRegisterMonoMethodType<StaticBodyComponentInterface::HasComponent>(static_body_class, "HasComponent", StaticBodyComponentInterface::HasComponent);
	mono_core->HookAndRegisterMonoMethodType<StaticBodyComponentInterface::RemoveComponent>(static_body_class, "RemoveComponent", StaticBodyComponentInterface::RemoveComponent);

	mono_core->HookAndRegisterMonoMethodType<StaticBodyComponentInterface::SetEnabled>(static_body_class, "SetEnabled", StaticBodyComponentInterface::SetEnabled);

	SceneLoader::Get()->OverrideSaveComponentMethod<StaticBodyComponent>(SaveScriptComponent, LoadScriptComponent);

	s_add_physic_object_index = add_physic_object_index;
	s_remove_physic_object_index = remove_physic_object_index;
}

void StaticBodyComponentInterface::InitComponent(const CSMonoObject& object, SceneIndex scene_index, Entity entity)
{
	SceneLoaderDeferCalls* defer_method_calls = SceneLoader::Get()->GetDeferedCalls();

	if (!defer_method_calls->TryCallDirectly(scene_index, s_add_physic_object_index, scene_index, entity, PhysicsCore::StaticBody))
	{
		SceneManager::GetSceneManager()->GetEntityManager(scene_index)->AddComponent<StaticBodyComponent>(entity);
	}
}

bool StaticBodyComponentInterface::HasComponent(const CSMonoObject& object, SceneIndex scene_index, Entity entity)
{
	return SceneManager::GetSceneManager()->GetEntityManager(scene_index)->HasComponent<StaticBodyComponent>(entity);
}

void StaticBodyComponentInterface::RemoveComponent(const CSMonoObject& object, SceneIndex scene_index, Entity entity)
{
	SceneLoaderDeferCalls* defer_method_calls = SceneLoader::Get()->GetDeferedCalls();

	defer_method_calls->TryCallDirectly(scene_index, s_remove_physic_object_index, scene_index, entity);
}

void StaticBodyComponentInterface::SetEnabled(const CSMonoObject& object, const bool enabled)
{
	const CSMonoObject game_object = ComponentInterface::GetGameObject(object);
	const Entity entity = GameObjectInterface::GetEntityID(game_object);
	const SceneIndex scene_index = GameObjectInterface::GetSceneIndex(game_object);
	SceneManager::GetSceneManager()->GetEntityManager(scene_index)->GetComponent<StaticBodyComponent>(entity).enabled = enabled;
}

void StaticBodyComponentInterface::SaveScriptComponent(Entity ent, EntityManager* entman, JsonObject* json_object)
{
}

void StaticBodyComponentInterface::LoadScriptComponent(Entity ent, EntityManager* entman, JsonObject* json_object)
{
}
