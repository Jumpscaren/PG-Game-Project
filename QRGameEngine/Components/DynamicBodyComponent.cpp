#include "pch.h"
#include "DynamicBodyComponent.h"
#include "Scripting/CSMonoCore.h"
#include "SceneSystem/SceneManager.h"
#include "ECS/EntityManager.h"
#include "Physics/PhysicsCore.h"
#include "SceneSystem/SceneLoader.h"
#include "Scripting/Objects/GameObjectInterface.h"
#include "ComponentInterface.h"
#include "Scripting/Objects/Vector2Interface.h"

void DynamicBodyComponentInterface::RegisterInterface(CSMonoCore* mono_core)
{
	auto dynamic_body_class = mono_core->RegisterMonoClass("ScriptProject.Engine", "DynamicBody");

	mono_core->HookAndRegisterMonoMethodType<DynamicBodyComponentInterface::InitComponent>(dynamic_body_class, "InitComponent", DynamicBodyComponentInterface::InitComponent);
	mono_core->HookAndRegisterMonoMethodType<DynamicBodyComponentInterface::HasComponent>(dynamic_body_class, "HasComponent", DynamicBodyComponentInterface::HasComponent);
	mono_core->HookAndRegisterMonoMethodType<DynamicBodyComponentInterface::RemoveComponent>(dynamic_body_class, "RemoveComponent", DynamicBodyComponentInterface::RemoveComponent);

	mono_core->HookAndRegisterMonoMethodType<DynamicBodyComponentInterface::SetVelocity>(dynamic_body_class, "SetVelocity_Extern", DynamicBodyComponentInterface::SetVelocity);
	mono_core->HookAndRegisterMonoMethodType<DynamicBodyComponentInterface::GetVelocity>(dynamic_body_class, "GetVelocity_Extern", DynamicBodyComponentInterface::GetVelocity);
	mono_core->HookAndRegisterMonoMethodType<DynamicBodyComponentInterface::SetFixedRotation>(dynamic_body_class, "SetFixedRotation", DynamicBodyComponentInterface::SetFixedRotation);
	mono_core->HookAndRegisterMonoMethodType<DynamicBodyComponentInterface::SetEnabled>(dynamic_body_class, "SetEnabled", DynamicBodyComponentInterface::SetEnabled);

	SceneLoader::Get()->OverrideSaveComponentMethod<DynamicBodyComponent>(SaveScriptComponent, LoadScriptComponent);
}

void DynamicBodyComponentInterface::InitComponent(const CSMonoObject& object, SceneIndex scene_index, Entity entity)
{
	PhysicsCore::Get()->AddPhysicObject(scene_index, entity, PhysicsCore::DynamicBody);
}

bool DynamicBodyComponentInterface::HasComponent(const CSMonoObject& object, SceneIndex scene_index, Entity entity)
{
	return SceneManager::GetSceneManager()->GetEntityManager(scene_index)->HasComponent<DynamicBodyComponent>(entity);
}

void DynamicBodyComponentInterface::RemoveComponent(const CSMonoObject& object, SceneIndex scene_index, Entity entity)
{
	PhysicsCore::Get()->RemovePhysicObject(scene_index, entity);
}

void DynamicBodyComponentInterface::SetVelocity(SceneIndex scene_index, Entity entity, const CSMonoObject& velocity)
{
	SceneManager::GetSceneManager()->GetEntityManager(scene_index)->GetComponent<DynamicBodyComponent>(entity).velocity = Vector2Interface::GetVector2(velocity);
}

CSMonoObject DynamicBodyComponentInterface::GetVelocity(SceneIndex scene_index, Entity entity)
{
	return Vector2Interface::CreateVector2(SceneManager::GetSceneManager()->GetEntityManager(scene_index)->GetComponent<DynamicBodyComponent>(entity).velocity);
}

void DynamicBodyComponentInterface::SetFixedRotation(const CSMonoObject& object, bool fixed_rotation)
{
	const CSMonoObject game_object = ComponentInterface::GetGameObject(object);
	Entity entity = GameObjectInterface::GetEntityID(game_object);
	SceneIndex scene_index = GameObjectInterface::GetSceneIndex(game_object);
	SceneManager::GetSceneManager()->GetEntityManager(scene_index)->GetComponent<DynamicBodyComponent>(entity).fixed_rotation = fixed_rotation;
}

void DynamicBodyComponentInterface::SetEnabled(const CSMonoObject& object, const bool enabled)
{
	const CSMonoObject game_object = ComponentInterface::GetGameObject(object);
	Entity entity = GameObjectInterface::GetEntityID(game_object);
	SceneIndex scene_index = GameObjectInterface::GetSceneIndex(game_object);
	SceneManager::GetSceneManager()->GetEntityManager(scene_index)->GetComponent<DynamicBodyComponent>(entity).enabled = enabled;
}

void DynamicBodyComponentInterface::SaveScriptComponent(Entity ent, EntityManager* entman, JsonObject* json_object)
{
}

void DynamicBodyComponentInterface::LoadScriptComponent(Entity ent, EntityManager* entman, JsonObject* json_object)
{
}
