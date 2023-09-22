#include "pch.h"
#include "StaticBodyComponent.h"
#include "Scripting/CSMonoCore.h"
#include "SceneSystem/SceneManager.h"
#include "ECS/EntityManager.h"
#include "Physics/PhysicsCore.h"
#include "SceneSystem/SceneLoader.h"

void StaticBodyComponentInterface::RegisterInterface(CSMonoCore* mono_core)
{
	auto static_body_class = mono_core->RegisterMonoClass("ScriptProject.Engine", "StaticBody");

	mono_core->HookAndRegisterMonoMethodType<StaticBodyComponentInterface::InitComponent>(static_body_class, "InitComponent", StaticBodyComponentInterface::InitComponent);
	mono_core->HookAndRegisterMonoMethodType<StaticBodyComponentInterface::HasComponent>(static_body_class, "HasComponent", StaticBodyComponentInterface::HasComponent);
	mono_core->HookAndRegisterMonoMethodType<StaticBodyComponentInterface::RemoveComponent>(static_body_class, "RemoveComponent", StaticBodyComponentInterface::RemoveComponent);

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

void StaticBodyComponentInterface::SaveScriptComponent(Entity ent, EntityManager* entman, OutputFile* file)
{
}

void StaticBodyComponentInterface::LoadScriptComponent(Entity ent, EntityManager* entman, OutputFile* file)
{
}
