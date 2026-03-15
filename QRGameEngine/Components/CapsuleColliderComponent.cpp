#include "pch.h"
#include "CapsuleColliderComponent.h"
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
#include "Scripting/Objects/Vector2Interface.h"

DeferedMethodIndex CapsuleColliderComponentInterface::s_add_capsule_collider_index;
DeferedMethodIndex CapsuleColliderComponentInterface::s_add_physic_object_index;
DeferedMethodIndex CapsuleColliderComponentInterface::s_remove_capsule_collider_index;

void CapsuleColliderComponentInterface::RegisterInterface(CSMonoCore* mono_core, const DeferedMethodIndex add_physic_object_index, const DeferedMethodIndex add_capsule_collider_index, const DeferedMethodIndex remove_capsule_collider_index)
{
	auto capsule_collider_class = mono_core->RegisterMonoClass("ScriptProject.Engine", "CapsuleCollider");

	mono_core->HookAndRegisterMonoMethodType<CapsuleColliderComponentInterface::InitComponent>(capsule_collider_class, "InitComponent", CapsuleColliderComponentInterface::InitComponent);
	mono_core->HookAndRegisterMonoMethodType<CapsuleColliderComponentInterface::HasComponent>(capsule_collider_class, "HasComponent", CapsuleColliderComponentInterface::HasComponent);
	mono_core->HookAndRegisterMonoMethodType<CapsuleColliderComponentInterface::RemoveComponent>(capsule_collider_class, "RemoveComponent", CapsuleColliderComponentInterface::RemoveComponent);

	mono_core->HookAndRegisterMonoMethodType<CapsuleColliderComponentInterface::SetTrigger>(capsule_collider_class, "SetTrigger", CapsuleColliderComponentInterface::SetTrigger);
	mono_core->HookAndRegisterMonoMethodType<CapsuleColliderComponentInterface::SetColliderFilter>(capsule_collider_class, "SetColliderFilter", CapsuleColliderComponentInterface::SetColliderFilter);
	mono_core->HookAndRegisterMonoMethodType<CapsuleColliderComponentInterface::SetPoints>(capsule_collider_class, "SetPoints", CapsuleColliderComponentInterface::SetPoints);
	mono_core->HookAndRegisterMonoMethodType<CapsuleColliderComponentInterface::SetRadius>(capsule_collider_class, "SetRadius", CapsuleColliderComponentInterface::SetRadius);

	SceneLoader::Get()->OverrideSaveComponentMethod<CapsuleColliderComponent>(SaveScriptComponent, LoadScriptComponent);

	s_add_physic_object_index = add_physic_object_index;
	s_add_capsule_collider_index = add_capsule_collider_index;
	s_remove_capsule_collider_index = remove_capsule_collider_index;
}

void CapsuleColliderComponentInterface::InitComponent(const CSMonoObject& object, SceneIndex scene_index, Entity entity)
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

	if (!defer_method_calls->TryCallDirectly(scene_index, s_add_capsule_collider_index, scene_index, entity, Vector2(), Vector2(0.0f, 1.0f), 0.5f, false, ColliderFilter{}))
	{
		SceneManager::GetSceneManager()->GetEntityManager(scene_index)->AddComponent<CapsuleColliderComponent>(entity).debug_draw = true;
	}
}

bool CapsuleColliderComponentInterface::HasComponent(const CSMonoObject& object, SceneIndex scene_index, Entity entity)
{
	return SceneManager::GetSceneManager()->GetEntityManager(scene_index)->HasComponent<CapsuleColliderComponent>(entity);
}

void CapsuleColliderComponentInterface::RemoveComponent(const CSMonoObject& object, SceneIndex scene_index, Entity entity)
{
	SceneLoaderDeferCalls* defer_method_calls = SceneLoader::Get()->GetDeferedCalls();

	defer_method_calls->TryCallDirectly(scene_index, s_remove_capsule_collider_index, scene_index, entity);
}

void CapsuleColliderComponentInterface::SetTrigger(const CSMonoObject& object, const bool trigger)
{
	const CSMonoObject game_object = GameObjectInterface::GetGameObjectFromComponent(object);
	const auto scene_index = GameObjectInterface::GetSceneIndex(game_object);
	const auto entity = GameObjectInterface::GetEntityID(game_object);
	CapsuleColliderComponent& capsule_collider = SceneManager::GetSceneManager()->GetEntityManager(scene_index)->GetComponent<CapsuleColliderComponent>(entity);
	capsule_collider.trigger = trigger;
	capsule_collider.update_collider = true;
}

void CapsuleColliderComponentInterface::SetColliderFilter(const CSMonoObject& object, const uint16_t category, const uint16_t mask, const int16_t group_index)
{
	const CSMonoObject game_object = GameObjectInterface::GetGameObjectFromComponent(object);
	const auto scene_index = GameObjectInterface::GetSceneIndex(game_object);
	const auto entity = GameObjectInterface::GetEntityID(game_object);
	CapsuleColliderComponent& capsule_collider = SceneManager::GetSceneManager()->GetEntityManager(scene_index)->GetComponent<CapsuleColliderComponent>(entity);
	capsule_collider.filter.category_bits = category;
	capsule_collider.filter.mask_bits = mask;
	capsule_collider.filter.group_index = group_index;
	capsule_collider.update_collider = true;
}

void CapsuleColliderComponentInterface::SetPoints(const CSMonoObject& object, const CSMonoObject& point_1, const CSMonoObject& point_2)
{
	const CSMonoObject game_object = GameObjectInterface::GetGameObjectFromComponent(object);
	const auto scene_index = GameObjectInterface::GetSceneIndex(game_object);
	const auto entity = GameObjectInterface::GetEntityID(game_object);

	CapsuleColliderComponent& capsule_collider = SceneManager::GetSceneManager()->GetEntityManager(scene_index)->GetComponent<CapsuleColliderComponent>(entity);
	capsule_collider.point_1 = Vector2Interface::GetVector2(point_1);
	capsule_collider.point_2 = Vector2Interface::GetVector2(point_2);
	capsule_collider.update_collider = true;
}

void CapsuleColliderComponentInterface::SetRadius(const CSMonoObject& object, float radius)
{
	const CSMonoObject game_object = GameObjectInterface::GetGameObjectFromComponent(object);
	const auto scene_index = GameObjectInterface::GetSceneIndex(game_object);
	const auto entity = GameObjectInterface::GetEntityID(game_object);
	CapsuleColliderComponent& capsule_collider = SceneManager::GetSceneManager()->GetEntityManager(scene_index)->GetComponent<CapsuleColliderComponent>(entity);
	capsule_collider.radius = radius;
	capsule_collider.update_collider = true;
}

void CapsuleColliderComponentInterface::SaveScriptComponent(Entity ent, EntityManager* entman, JsonObject* json_object)
{
	const CapsuleColliderComponent& capsule_collider = entman->GetComponent<CapsuleColliderComponent>(ent);
	json_object->SetData(capsule_collider.trigger, "trigger");
	json_object->SetData(capsule_collider.radius, "radius");
	json_object->SetData(capsule_collider.point_1, "point_1");
	json_object->SetData(capsule_collider.point_2, "point_2");

	json_object->SetData(capsule_collider.filter.category_bits, "filter.category_bits");
	json_object->SetData(capsule_collider.filter.mask_bits, "filter.mask_bits");
	json_object->SetData(capsule_collider.filter.group_index, "filter.group_index");
}

void CapsuleColliderComponentInterface::LoadScriptComponent(Entity ent, EntityManager* entman, JsonObject* json_object)
{
	CapsuleColliderComponent& capsule_collider = entman->GetComponent<CapsuleColliderComponent>(ent);
	json_object->LoadData(capsule_collider.trigger, "trigger");
	json_object->LoadData(capsule_collider.radius, "radius");
	json_object->LoadData(capsule_collider.point_1, "point_1");
	json_object->LoadData(capsule_collider.point_2, "point_2");

	json_object->LoadData(capsule_collider.filter.category_bits, "filter.category_bits");
	json_object->LoadData(capsule_collider.filter.mask_bits, "filter.mask_bits");
	json_object->LoadData(capsule_collider.filter.group_index, "filter.group_index");

	capsule_collider.update_collider = true;
}
