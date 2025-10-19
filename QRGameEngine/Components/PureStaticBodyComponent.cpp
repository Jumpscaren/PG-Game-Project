#include "pch.h"
#include "PureStaticBodyComponent.h"
#include "Scripting/CSMonoCore.h"
#include "SceneSystem/SceneManager.h"
#include "ECS/EntityManager.h"
#include "Physics/PhysicsCore.h"
#include "SceneSystem/SceneLoader.h"
#include "ComponentInterface.h"
#include "Scripting/Objects/GameObjectInterface.h"

DeferedMethodIndex PureStaticBodyComponentInterface::s_add_physic_object_index;
DeferedMethodIndex PureStaticBodyComponentInterface::s_remove_physic_object_index;

void PureStaticBodyComponentInterface::RegisterInterface(CSMonoCore* mono_core, const DeferedMethodIndex add_physic_object_index, const DeferedMethodIndex remove_physic_object_index)
{
	const auto static_body_class = mono_core->RegisterMonoClass("ScriptProject.Engine", "PureStaticBody");

	mono_core->HookAndRegisterMonoMethodType<PureStaticBodyComponentInterface::InitComponent>(static_body_class, "InitComponent", PureStaticBodyComponentInterface::InitComponent);
	mono_core->HookAndRegisterMonoMethodType<PureStaticBodyComponentInterface::HasComponent>(static_body_class, "HasComponent", PureStaticBodyComponentInterface::HasComponent);
	mono_core->HookAndRegisterMonoMethodType<PureStaticBodyComponentInterface::RemoveComponent>(static_body_class, "RemoveComponent", PureStaticBodyComponentInterface::RemoveComponent);

	SceneLoader::Get()->OverrideSaveComponentMethod<PureStaticBodyComponent>(SaveScriptComponent, LoadScriptComponent);

	s_add_physic_object_index = add_physic_object_index;
	s_remove_physic_object_index = remove_physic_object_index;
}

void PureStaticBodyComponentInterface::InitComponent(const CSMonoObject& object, SceneIndex scene_index, Entity entity)
{
	SceneLoaderDeferCalls* defer_method_calls = SceneLoader::Get()->GetDeferedCalls();

	if (!defer_method_calls->TryCallDirectly(scene_index, s_add_physic_object_index, scene_index, entity, PhysicsCore::PureStaticBody))
	{
		SceneManager::GetSceneManager()->GetEntityManager(scene_index)->AddComponent<PureStaticBodyComponent>(entity);
	}
}

bool PureStaticBodyComponentInterface::HasComponent(const CSMonoObject& object, SceneIndex scene_index, Entity entity)
{
	return SceneManager::GetSceneManager()->GetEntityManager(scene_index)->HasComponent<PureStaticBodyComponent>(entity);
}

void PureStaticBodyComponentInterface::RemoveComponent(const CSMonoObject& object, SceneIndex scene_index, Entity entity)
{
	SceneLoaderDeferCalls* defer_method_calls = SceneLoader::Get()->GetDeferedCalls();

	defer_method_calls->TryCallDirectly(scene_index, s_remove_physic_object_index, scene_index, entity);
}

void PureStaticBodyComponentInterface::SaveScriptComponent(Entity ent, EntityManager* entman, JsonObject* json_object)
{
}

void PureStaticBodyComponentInterface::LoadScriptComponent(Entity ent, EntityManager* entman, JsonObject* json_object)
{
}