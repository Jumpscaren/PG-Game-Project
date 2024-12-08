#include "pch.h"
#include "KinematicBodyComponent.h"
#include "Scripting/CSMonoCore.h"
#include "SceneSystem/SceneManager.h"
#include "ECS/EntityManager.h"
#include "Physics/PhysicsCore.h"
#include "SceneSystem/SceneLoader.h"
#include "Scripting/Objects/GameObjectInterface.h"
#include "ComponentInterface.h"
#include "Scripting/Objects/Vector2Interface.h"

void KinematicBodyComponentInterface::RegisterInterface(CSMonoCore* mono_core)
{
	auto kinematic_body_class = mono_core->RegisterMonoClass("ScriptProject.Engine", "KinematicBody");

	mono_core->HookAndRegisterMonoMethodType<KinematicBodyComponentInterface::InitComponent>(kinematic_body_class, "InitComponent", KinematicBodyComponentInterface::InitComponent);
	mono_core->HookAndRegisterMonoMethodType<KinematicBodyComponentInterface::HasComponent>(kinematic_body_class, "HasComponent", KinematicBodyComponentInterface::HasComponent);
	mono_core->HookAndRegisterMonoMethodType<KinematicBodyComponentInterface::RemoveComponent>(kinematic_body_class, "RemoveComponent", KinematicBodyComponentInterface::RemoveComponent);

	mono_core->HookAndRegisterMonoMethodType<KinematicBodyComponentInterface::SetVelocity>(kinematic_body_class, "SetVelocity", KinematicBodyComponentInterface::SetVelocity);
	mono_core->HookAndRegisterMonoMethodType<KinematicBodyComponentInterface::GetVelocity>(kinematic_body_class, "GetVelocity", KinematicBodyComponentInterface::GetVelocity);
	mono_core->HookAndRegisterMonoMethodType<KinematicBodyComponentInterface::SetFixedRotation>(kinematic_body_class, "SetFixedRotation", KinematicBodyComponentInterface::SetFixedRotation);
	mono_core->HookAndRegisterMonoMethodType<KinematicBodyComponentInterface::SetEnabled>(kinematic_body_class, "SetEnabled", KinematicBodyComponentInterface::SetEnabled);

	SceneLoader::Get()->OverrideSaveComponentMethod<KinematicBodyComponent>(SaveScriptComponent, LoadScriptComponent);
}

void KinematicBodyComponentInterface::InitComponent(const CSMonoObject& object, SceneIndex scene_index, Entity entity)
{
	PhysicsCore::Get()->AddPhysicObject(scene_index, entity, PhysicsCore::KinematicBody);
}

bool KinematicBodyComponentInterface::HasComponent(const CSMonoObject& object, SceneIndex scene_index, Entity entity)
{
	return SceneManager::GetSceneManager()->GetEntityManager(scene_index)->HasComponent<KinematicBodyComponent>(entity);
}

void KinematicBodyComponentInterface::RemoveComponent(const CSMonoObject& object, SceneIndex scene_index, Entity entity)
{
	PhysicsCore::Get()->RemovePhysicObject(scene_index, entity);
}

void KinematicBodyComponentInterface::SetVelocity(const CSMonoObject& object, const CSMonoObject& velocity)
{
	const CSMonoObject game_object = ComponentInterface::GetGameObject(object);
	Entity entity = GameObjectInterface::GetEntityID(game_object);
	SceneIndex scene_index = GameObjectInterface::GetSceneIndex(game_object);
	SceneManager::GetSceneManager()->GetEntityManager(scene_index)->GetComponent<KinematicBodyComponent>(entity).velocity = Vector2Interface::GetVector2(velocity);
}

CSMonoObject KinematicBodyComponentInterface::GetVelocity(const CSMonoObject& object)
{
	const CSMonoObject game_object = ComponentInterface::GetGameObject(object);
	Entity entity = GameObjectInterface::GetEntityID(game_object);
	SceneIndex scene_index = GameObjectInterface::GetSceneIndex(game_object);
	return Vector2Interface::CreateVector2(SceneManager::GetSceneManager()->GetEntityManager(scene_index)->GetComponent<KinematicBodyComponent>(entity).velocity);
}

void KinematicBodyComponentInterface::SetFixedRotation(const CSMonoObject& object, bool fixed_rotation)
{
	const CSMonoObject game_object = ComponentInterface::GetGameObject(object);
	Entity entity = GameObjectInterface::GetEntityID(game_object);
	SceneIndex scene_index = GameObjectInterface::GetSceneIndex(game_object);
	SceneManager::GetSceneManager()->GetEntityManager(scene_index)->GetComponent<KinematicBodyComponent>(entity).fixed_rotation = fixed_rotation;
}

void KinematicBodyComponentInterface::SetEnabled(const CSMonoObject& object, const bool enabled)
{
	const CSMonoObject game_object = ComponentInterface::GetGameObject(object);
	Entity entity = GameObjectInterface::GetEntityID(game_object);
	SceneIndex scene_index = GameObjectInterface::GetSceneIndex(game_object);
	SceneManager::GetSceneManager()->GetEntityManager(scene_index)->GetComponent<KinematicBodyComponent>(entity).enabled = enabled;
}

void KinematicBodyComponentInterface::SaveScriptComponent(Entity ent, EntityManager* entman, JsonObject* json_object)
{
}

void KinematicBodyComponentInterface::LoadScriptComponent(Entity ent, EntityManager* entman, JsonObject* json_object)
{
}
