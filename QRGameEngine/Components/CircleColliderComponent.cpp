#include "pch.h"
#include "CircleColliderComponent.h"
#include "Scripting/CSMonoCore.h"
#include "SceneSystem/SceneManager.h"
#include "ECS/EntityManager.h"
#include "Physics/PhysicsCore.h"
#include "SceneSystem/SceneLoader.h"
#include "DynamicBodyComponent.h"
#include "StaticBodyComponent.h"
#include "PureStaticBodyComponent.h"
#include "IO/JsonObject.h"
#include "Scripting/Objects/GameObjectInterface.h"
#include "KinematicBodyComponent.h"

DeferedMethodIndex CircleColliderComponentInterface::s_add_circle_collider_index;
DeferedMethodIndex CircleColliderComponentInterface::s_add_physic_object_index;
DeferedMethodIndex CircleColliderComponentInterface::s_remove_circle_collider_index;

void CircleColliderComponentInterface::RegisterInterface(CSMonoCore* mono_core, const DeferedMethodIndex add_physic_object_index, const DeferedMethodIndex add_circle_collider_index, const DeferedMethodIndex remove_circle_collider_index)
{
	auto circle_collider_class = mono_core->RegisterMonoClass("ScriptProject.Engine", "CircleCollider");

	mono_core->HookAndRegisterMonoMethodType<CircleColliderComponentInterface::InitComponent>(circle_collider_class, "InitComponent", CircleColliderComponentInterface::InitComponent);
	mono_core->HookAndRegisterMonoMethodType<CircleColliderComponentInterface::HasComponent>(circle_collider_class, "HasComponent", CircleColliderComponentInterface::HasComponent);
	mono_core->HookAndRegisterMonoMethodType<CircleColliderComponentInterface::RemoveComponent>(circle_collider_class, "RemoveComponent", CircleColliderComponentInterface::RemoveComponent);

	mono_core->HookAndRegisterMonoMethodType<CircleColliderComponentInterface::SetTrigger>(circle_collider_class, "SetTrigger", CircleColliderComponentInterface::SetTrigger);
	mono_core->HookAndRegisterMonoMethodType<CircleColliderComponentInterface::SetColliderFilter>(circle_collider_class, "SetColliderFilter", CircleColliderComponentInterface::SetColliderFilter);
	mono_core->HookAndRegisterMonoMethodType<CircleColliderComponentInterface::SetRadius>(circle_collider_class, "SetRadius", CircleColliderComponentInterface::SetRadius);

	SceneLoader::Get()->OverrideSaveComponentMethod<CircleColliderComponent>(SaveScriptComponent, LoadScriptComponent);

	s_add_physic_object_index = add_physic_object_index;
	s_add_circle_collider_index = add_circle_collider_index;
	s_remove_circle_collider_index = remove_circle_collider_index;
}

void CircleColliderComponentInterface::InitComponent(const CSMonoObject& object, SceneIndex scene_index, Entity entity)
{
	EntityManager* entity_manager = SceneManager::GetSceneManager()->GetEntityManager(scene_index);
	SceneLoaderDeferCalls* defer_method_calls = SceneLoader::Get()->GetDeferedCalls();

	//So that we do not need add staticbody when adding a circlecollider if we do not use a dynamic body
	if (!entity_manager->HasComponent<DynamicBodyComponent>(entity) && !entity_manager->HasComponent<StaticBodyComponent>(entity) && !entity_manager->HasComponent<PureStaticBodyComponent>(entity) && !entity_manager->HasComponent<KinematicBodyComponent>(entity))
	{
		if (!defer_method_calls->TryCallDirectly(scene_index, s_add_physic_object_index, scene_index, entity, PhysicsCore::StaticBody))
		{
			SceneManager::GetSceneManager()->GetEntityManager(scene_index)->AddComponent<StaticBodyComponent>(entity);
		}
	}

	if (!defer_method_calls->TryCallDirectly(scene_index, s_add_circle_collider_index, scene_index, entity, 0.5f, false, ColliderFilter{}))
	{
		SceneManager::GetSceneManager()->GetEntityManager(scene_index)->AddComponent<CircleColliderComponent>(entity).debug_draw = true;
	}
}

bool CircleColliderComponentInterface::HasComponent(const CSMonoObject& object, SceneIndex scene_index, Entity entity)
{
	return SceneManager::GetSceneManager()->GetEntityManager(scene_index)->HasComponent<CircleColliderComponent>(entity);
}

void CircleColliderComponentInterface::RemoveComponent(const CSMonoObject& object, SceneIndex scene_index, Entity entity)
{
	SceneLoaderDeferCalls* defer_method_calls = SceneLoader::Get()->GetDeferedCalls();

	defer_method_calls->TryCallDirectly(scene_index, s_remove_circle_collider_index, scene_index, entity);
}

void CircleColliderComponentInterface::SetTrigger(const CSMonoObject& object, const bool trigger)
{
	const CSMonoObject game_object = GameObjectInterface::GetGameObjectFromComponent(object);
	const auto scene_index = GameObjectInterface::GetSceneIndex(game_object);
	const auto entity = GameObjectInterface::GetEntityID(game_object);
	CircleColliderComponent& circle_collider = SceneManager::GetSceneManager()->GetEntityManager(scene_index)->GetComponent<CircleColliderComponent>(entity);
	circle_collider.trigger = trigger;
	circle_collider.update_circle_collider = true;
}

void CircleColliderComponentInterface::SetColliderFilter(const CSMonoObject& object, const uint16_t category, const uint16_t mask, const int16_t group_index)
{
	const CSMonoObject game_object = GameObjectInterface::GetGameObjectFromComponent(object);
	const auto scene_index = GameObjectInterface::GetSceneIndex(game_object);
	const auto entity = GameObjectInterface::GetEntityID(game_object);
	CircleColliderComponent& circle_collider = SceneManager::GetSceneManager()->GetEntityManager(scene_index)->GetComponent<CircleColliderComponent>(entity);
	circle_collider.filter.category_bits = category;
	circle_collider.filter.mask_bits = mask;
	circle_collider.filter.group_index = group_index;
	circle_collider.update_circle_collider = true;
}

void CircleColliderComponentInterface::SetRadius(const CSMonoObject& object, float radius)
{
	const CSMonoObject game_object = GameObjectInterface::GetGameObjectFromComponent(object);
	const auto scene_index = GameObjectInterface::GetSceneIndex(game_object);
	const auto entity = GameObjectInterface::GetEntityID(game_object);
	CircleColliderComponent& circle_collider = SceneManager::GetSceneManager()->GetEntityManager(scene_index)->GetComponent<CircleColliderComponent>(entity);
	circle_collider.circle_radius = radius;
	circle_collider.update_circle_collider = true;
}

void CircleColliderComponentInterface::SaveScriptComponent(Entity ent, EntityManager* entman, JsonObject* json_object)
{
	const CircleColliderComponent& circle_collider = entman->GetComponent<CircleColliderComponent>(ent);
	json_object->SetData(circle_collider.trigger, "trigger");
	json_object->SetData(circle_collider.circle_radius, "circle_radius");

	json_object->SetData(circle_collider.filter.category_bits, "filter.category_bits");
	json_object->SetData(circle_collider.filter.mask_bits, "filter.mask_bits");
	json_object->SetData(circle_collider.filter.group_index, "filter.group_index");
}

void CircleColliderComponentInterface::LoadScriptComponent(Entity ent, EntityManager* entman, JsonObject* json_object)
{
	CircleColliderComponent& circle_collider = entman->GetComponent<CircleColliderComponent>(ent);
	json_object->LoadData(circle_collider.trigger, "trigger");
	json_object->LoadData(circle_collider.circle_radius, "circle_radius");

	json_object->LoadData(circle_collider.filter.category_bits, "filter.category_bits");
	json_object->LoadData(circle_collider.filter.mask_bits, "filter.mask_bits");
	json_object->LoadData(circle_collider.filter.group_index, "filter.group_index");

	circle_collider.update_circle_collider = true;
}
