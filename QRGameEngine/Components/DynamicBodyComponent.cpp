#include "pch.h"
#include "DynamicBodyComponent.h"
#include "Scripting/CSMonoCore.h"
#include "SceneSystem/SceneManager.h"
#include "ECS/EntityManager.h"
#include "Physics/PhysicsCore.h"
#include "SceneSystem/SceneLoader.h"

void DynamicBodyComponentInterface::RegisterInterface(CSMonoCore* mono_core)
{
	auto dynamic_body_class = mono_core->RegisterMonoClass("ScriptProject.Engine", "DynamicBody");

	mono_core->HookAndRegisterMonoMethodType<DynamicBodyComponentInterface::InitComponent>(dynamic_body_class, "InitComponent", DynamicBodyComponentInterface::InitComponent);
	mono_core->HookAndRegisterMonoMethodType<DynamicBodyComponentInterface::HasComponent>(dynamic_body_class, "HasComponent", DynamicBodyComponentInterface::HasComponent);
	mono_core->HookAndRegisterMonoMethodType<DynamicBodyComponentInterface::RemoveComponent>(dynamic_body_class, "RemoveComponent", DynamicBodyComponentInterface::RemoveComponent);

	SceneLoader::Get()->OverrideSaveComponentMethod<DynamicBodyComponent>(SaveScriptComponent, LoadScriptComponent);
}

void DynamicBodyComponentInterface::InitComponent(CSMonoObject object, SceneIndex scene_index, Entity entity)
{
	PhysicsCore::Get()->AddPhysicObject(scene_index, entity, PhysicsCore::DynamicBody);
}

bool DynamicBodyComponentInterface::HasComponent(CSMonoObject object, SceneIndex scene_index, Entity entity)
{
	return SceneManager::GetSceneManager()->GetEntityManager(scene_index)->HasComponent<DynamicBodyComponent>(entity);
}

void DynamicBodyComponentInterface::RemoveComponent(CSMonoObject object, SceneIndex scene_index, Entity entity)
{
	PhysicsCore::Get()->RemovePhysicObject(scene_index, entity);
}

void DynamicBodyComponentInterface::SaveScriptComponent(Entity ent, EntityManager* entman, JsonObject* json_object)
{
}

void DynamicBodyComponentInterface::LoadScriptComponent(Entity ent, EntityManager* entman, JsonObject* json_object)
{
}
