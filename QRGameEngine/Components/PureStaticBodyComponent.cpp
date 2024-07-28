#include "pch.h"
#include "PureStaticBodyComponent.h"
#include "Scripting/CSMonoCore.h"
#include "SceneSystem/SceneManager.h"
#include "ECS/EntityManager.h"
#include "Physics/PhysicsCore.h"
#include "SceneSystem/SceneLoader.h"
#include "ComponentInterface.h"
#include "Scripting/Objects/GameObjectInterface.h"

void PureStaticBodyComponentInterface::RegisterInterface(CSMonoCore* mono_core)
{
	const auto static_body_class = mono_core->RegisterMonoClass("ScriptProject.Engine", "PureStaticBody");

	mono_core->HookAndRegisterMonoMethodType<PureStaticBodyComponentInterface::InitComponent>(static_body_class, "InitComponent", PureStaticBodyComponentInterface::InitComponent);
	mono_core->HookAndRegisterMonoMethodType<PureStaticBodyComponentInterface::HasComponent>(static_body_class, "HasComponent", PureStaticBodyComponentInterface::HasComponent);
	mono_core->HookAndRegisterMonoMethodType<PureStaticBodyComponentInterface::RemoveComponent>(static_body_class, "RemoveComponent", PureStaticBodyComponentInterface::RemoveComponent);

	SceneLoader::Get()->OverrideSaveComponentMethod<PureStaticBodyComponent>(SaveScriptComponent, LoadScriptComponent);
}

void PureStaticBodyComponentInterface::InitComponent(const CSMonoObject& object, SceneIndex scene_index, Entity entity)
{
	PhysicsCore::Get()->AddPhysicObject(scene_index, entity, PhysicsCore::PureStaticBody);
}

bool PureStaticBodyComponentInterface::HasComponent(const CSMonoObject& object, SceneIndex scene_index, Entity entity)
{
	return SceneManager::GetSceneManager()->GetEntityManager(scene_index)->HasComponent<PureStaticBodyComponent>(entity);
}

void PureStaticBodyComponentInterface::RemoveComponent(const CSMonoObject& object, SceneIndex scene_index, Entity entity)
{
	PhysicsCore::Get()->RemovePhysicObject(scene_index, entity);
}

void PureStaticBodyComponentInterface::SaveScriptComponent(Entity ent, EntityManager* entman, JsonObject* json_object)
{
}

void PureStaticBodyComponentInterface::LoadScriptComponent(Entity ent, EntityManager* entman, JsonObject* json_object)
{
}