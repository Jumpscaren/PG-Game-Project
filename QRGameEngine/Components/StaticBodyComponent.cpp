#include "pch.h"
#include "StaticBodyComponent.h"
#include "Scripting/CSMonoCore.h"
#include "SceneSystem/SceneManager.h"
#include "ECS/EntityManager.h"
#include "Physics/PhysicsCore.h"
#include "SceneSystem/SceneLoader.h"
#include "ComponentInterface.h"
#include "Scripting/Objects/GameObjectInterface.h"

void StaticBodyComponentInterface::RegisterInterface(CSMonoCore* mono_core)
{
	const auto static_body_class = mono_core->RegisterMonoClass("ScriptProject.Engine", "StaticBody");

	mono_core->HookAndRegisterMonoMethodType<StaticBodyComponentInterface::InitComponent>(static_body_class, "InitComponent", StaticBodyComponentInterface::InitComponent);
	mono_core->HookAndRegisterMonoMethodType<StaticBodyComponentInterface::HasComponent>(static_body_class, "HasComponent", StaticBodyComponentInterface::HasComponent);
	mono_core->HookAndRegisterMonoMethodType<StaticBodyComponentInterface::RemoveComponent>(static_body_class, "RemoveComponent", StaticBodyComponentInterface::RemoveComponent);

	mono_core->HookAndRegisterMonoMethodType<StaticBodyComponentInterface::SetEnabled>(static_body_class, "SetEnabled", StaticBodyComponentInterface::SetEnabled);

	SceneLoader::Get()->OverrideSaveComponentMethod<StaticBodyComponent>(SaveScriptComponent, LoadScriptComponent);
}

void StaticBodyComponentInterface::InitComponent(CSMonoObject object, SceneIndex scene_index, Entity entity)
{
	PhysicsCore::Get()->AddPhysicObject(scene_index, entity, PhysicsCore::StaticBody);
}

bool StaticBodyComponentInterface::HasComponent(CSMonoObject object, SceneIndex scene_index, Entity entity)
{
	return SceneManager::GetSceneManager()->GetEntityManager(scene_index)->HasComponent<StaticBodyComponent>(entity);
}

void StaticBodyComponentInterface::RemoveComponent(CSMonoObject object, SceneIndex scene_index, Entity entity)
{
	PhysicsCore::Get()->RemovePhysicObject(scene_index, entity);
}

void StaticBodyComponentInterface::SetEnabled(const CSMonoObject object, const bool enabled)
{
	const CSMonoObject& game_object = ComponentInterface::GetGameObject(object);
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
