#include "pch.h"
#include "BoxColliderComponent.h"
#include "Scripting/CSMonoCore.h"
#include "SceneSystem/SceneManager.h"
#include "ECS/EntityManager.h"
#include "Physics/PhysicsCore.h"
#include "SceneSystem/SceneLoader.h"
#include "DynamicBodyComponent.h"
#include "StaticBodyComponent.h"
#include "Scripting/Objects/GameObjectInterface.h"
#include "IO/JsonObject.h"

void BoxColliderComponentInterface::RegisterInterface(CSMonoCore* mono_core)
{
	auto box_collider_class = mono_core->RegisterMonoClass("ScriptProject.Engine", "BoxCollider");

	mono_core->HookAndRegisterMonoMethodType<BoxColliderComponentInterface::InitComponent>(box_collider_class, "InitComponent", BoxColliderComponentInterface::InitComponent);
	mono_core->HookAndRegisterMonoMethodType<BoxColliderComponentInterface::HasComponent>(box_collider_class, "HasComponent", BoxColliderComponentInterface::HasComponent);
	mono_core->HookAndRegisterMonoMethodType<BoxColliderComponentInterface::RemoveComponent>(box_collider_class, "RemoveComponent", BoxColliderComponentInterface::RemoveComponent);

	mono_core->HookAndRegisterMonoMethodType<BoxColliderComponentInterface::SetTrigger>(box_collider_class, "SetTrigger", BoxColliderComponentInterface::SetTrigger);

	SceneLoader::Get()->OverrideSaveComponentMethod<BoxColliderComponent>(SaveScriptComponent, LoadScriptComponent);
}

void BoxColliderComponentInterface::InitComponent(CSMonoObject object, SceneIndex scene_index, Entity entity)
{
	EntityManager* entity_manager = SceneManager::GetSceneManager()->GetEntityManager(scene_index);

	//So that we do not need add staticbody when adding a boxcollider if we do not use a dynamic body
	if (!entity_manager->HasComponent<DynamicBodyComponent>(entity) && !entity_manager->HasComponent<StaticBodyComponent>(entity))
		PhysicsCore::Get()->AddPhysicObject(scene_index, entity, PhysicsCore::StaticBody);

	PhysicsCore::Get()->AddBoxCollider(scene_index, entity, Vector2(0.5f, 0.5f));
}

bool BoxColliderComponentInterface::HasComponent(CSMonoObject object, SceneIndex scene_index, Entity entity)
{
	return SceneManager::GetSceneManager()->GetEntityManager(scene_index)->HasComponent<BoxColliderComponent>(entity);
}

void BoxColliderComponentInterface::RemoveComponent(CSMonoObject object, SceneIndex scene_index, Entity entity)
{
	PhysicsCore::Get()->RemoveBoxCollider(scene_index, entity);
}

void BoxColliderComponentInterface::SetTrigger(CSMonoObject object, bool trigger)
{
	const CSMonoObject game_object = GameObjectInterface::GetGameObjectFromComponent(object);
	const auto scene_index = GameObjectInterface::GetSceneIndex(game_object);
	const auto entity = GameObjectInterface::GetEntityID(game_object);
	BoxColliderComponent& box_collider = SceneManager::GetSceneManager()->GetEntityManager(scene_index)->GetComponent<BoxColliderComponent>(entity);
	box_collider.trigger = trigger;
	box_collider.update_box_collider = true;
}

void BoxColliderComponentInterface::SaveScriptComponent(Entity ent, EntityManager* entman, JsonObject* json_object)
{
	const BoxColliderComponent& box_collider = entman->GetComponent<BoxColliderComponent>(ent);
	json_object->SetData(box_collider.trigger, "trigger");
	json_object->SetData(box_collider.half_box_size, "half_box_size");
}

void BoxColliderComponentInterface::LoadScriptComponent(Entity ent, EntityManager* entman, JsonObject* json_object)
{
	BoxColliderComponent& box_collider = entman->GetComponent<BoxColliderComponent>(ent);
	Vector2 half_box_size;
	json_object->LoadData(box_collider.trigger, "trigger");
	json_object->LoadData(half_box_size, "half_box_size");
	if (half_box_size.x != 0.0f && half_box_size.y != 0.0f)
		box_collider.half_box_size = half_box_size;
	box_collider.update_box_collider = true;
}
