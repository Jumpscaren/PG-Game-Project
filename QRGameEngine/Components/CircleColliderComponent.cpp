#include "pch.h"
#include "CircleColliderComponent.h"
#include "Scripting/CSMonoCore.h"
#include "SceneSystem/SceneManager.h"
#include "ECS/EntityManager.h"
#include "Physics/PhysicsCore.h"
#include "SceneSystem/SceneLoader.h"
#include "DynamicBodyComponent.h"
#include "StaticBodyComponent.h"

void CircleColliderComponentInterface::RegisterInterface(CSMonoCore* mono_core)
{
	auto circle_collider_class = mono_core->RegisterMonoClass("ScriptProject.Engine", "CircleCollider");

	mono_core->HookAndRegisterMonoMethodType<CircleColliderComponentInterface::InitComponent>(circle_collider_class, "InitComponent", CircleColliderComponentInterface::InitComponent);
	mono_core->HookAndRegisterMonoMethodType<CircleColliderComponentInterface::HasComponent>(circle_collider_class, "HasComponent", CircleColliderComponentInterface::HasComponent);
	mono_core->HookAndRegisterMonoMethodType<CircleColliderComponentInterface::RemoveComponent>(circle_collider_class, "RemoveComponent", CircleColliderComponentInterface::RemoveComponent);

	SceneLoader::Get()->OverrideSaveComponentMethod<CircleColliderComponent>(SaveScriptComponent, LoadScriptComponent);
}

void CircleColliderComponentInterface::InitComponent(CSMonoObject object, SceneIndex scene_index, Entity entity)
{
	EntityManager* entity_manager = SceneManager::GetSceneManager()->GetEntityManager(scene_index);

	//So that we do not need add staticbody when adding a boxcollider if we do not use a dynamic body
	if (!entity_manager->HasComponent<DynamicBodyComponent>(entity) && !entity_manager->HasComponent<StaticBodyComponent>(entity))
		PhysicsCore::Get()->AddPhysicObject(scene_index, entity, PhysicsCore::StaticBody);

	PhysicsCore::Get()->AddCircleCollider(scene_index, entity, 0.5f);
}

bool CircleColliderComponentInterface::HasComponent(CSMonoObject object, SceneIndex scene_index, Entity entity)
{
	return SceneManager::GetSceneManager()->GetEntityManager(scene_index)->HasComponent<CircleColliderComponent>(entity);
}

void CircleColliderComponentInterface::RemoveComponent(CSMonoObject object, SceneIndex scene_index, Entity entity)
{
	PhysicsCore::Get()->RemoveCircleCollider(scene_index, entity);
}

void CircleColliderComponentInterface::SaveScriptComponent(Entity ent, EntityManager* entman, OutputFile* file)
{
}

void CircleColliderComponentInterface::LoadScriptComponent(Entity ent, EntityManager* entman, OutputFile* file)
{
}
