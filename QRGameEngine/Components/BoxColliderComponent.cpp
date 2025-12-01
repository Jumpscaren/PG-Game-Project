#include "pch.h"
#include "BoxColliderComponent.h"
#include "Scripting/CSMonoCore.h"
#include "SceneSystem/SceneManager.h"
#include "ECS/EntityManager.h"
#include "Physics/PhysicsCore.h"
#include "SceneSystem/SceneLoader.h"
#include "DynamicBodyComponent.h"
#include "StaticBodyComponent.h"
#include "PureStaticBodyComponent.h"
#include "Scripting/Objects/GameObjectInterface.h"
#include "IO/JsonObject.h"
#include "Scripting/Objects/Vector2Interface.h"
#include "KinematicBodyComponent.h"

DeferedMethodIndex BoxColliderComponentInterface::s_add_box_collider_index;
DeferedMethodIndex BoxColliderComponentInterface::s_add_physic_object_index;
DeferedMethodIndex BoxColliderComponentInterface::s_remove_box_collider_index;

void BoxColliderComponentInterface::RegisterInterface(CSMonoCore* mono_core, const DeferedMethodIndex add_physic_object_index, const DeferedMethodIndex add_box_collider_index, const DeferedMethodIndex remove_box_collider_index)
{
	auto box_collider_class = mono_core->RegisterMonoClass("ScriptProject.Engine", "BoxCollider");

	mono_core->HookAndRegisterMonoMethodType<BoxColliderComponentInterface::InitComponent>(box_collider_class, "InitComponent", BoxColliderComponentInterface::InitComponent);
	mono_core->HookAndRegisterMonoMethodType<BoxColliderComponentInterface::HasComponent>(box_collider_class, "HasComponent", BoxColliderComponentInterface::HasComponent);
	mono_core->HookAndRegisterMonoMethodType<BoxColliderComponentInterface::RemoveComponent>(box_collider_class, "RemoveComponent", BoxColliderComponentInterface::RemoveComponent);

	mono_core->HookAndRegisterMonoMethodType<BoxColliderComponentInterface::SetColliderFilter>(box_collider_class, "SetColliderFilter", BoxColliderComponentInterface::SetColliderFilter);
	mono_core->HookAndRegisterMonoMethodType<BoxColliderComponentInterface::SetTrigger>(box_collider_class, "SetTrigger", BoxColliderComponentInterface::SetTrigger);
	mono_core->HookAndRegisterMonoMethodType<BoxColliderComponentInterface::SetHalfBoxSize>(box_collider_class, "SetHalfBoxSize", BoxColliderComponentInterface::SetHalfBoxSize);
	mono_core->HookAndRegisterMonoMethodType<BoxColliderComponentInterface::SetOffset>(box_collider_class, "SetOffset", BoxColliderComponentInterface::SetOffset);

	SceneLoader::Get()->OverrideSaveComponentMethod<BoxColliderComponent>(SaveScriptComponent, LoadScriptComponent);

	s_add_physic_object_index = add_physic_object_index;
	s_add_box_collider_index = add_box_collider_index;
	s_remove_box_collider_index = remove_box_collider_index;
}

void BoxColliderComponentInterface::InitComponent(const CSMonoObject& object, SceneIndex scene_index, Entity entity)
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

	if (!defer_method_calls->TryCallDirectly(scene_index, s_add_box_collider_index, scene_index, entity, Vector2(0.5f, 0.5f), false, ColliderFilter{}))
	{
		SceneManager::GetSceneManager()->GetEntityManager(scene_index)->AddComponent<BoxColliderComponent>(entity).debug_draw = true;
	}
}

bool BoxColliderComponentInterface::HasComponent(const CSMonoObject& object, SceneIndex scene_index, Entity entity)
{
	return SceneManager::GetSceneManager()->GetEntityManager(scene_index)->HasComponent<BoxColliderComponent>(entity);
}

void BoxColliderComponentInterface::RemoveComponent(const CSMonoObject& object, SceneIndex scene_index, Entity entity)
{
	SceneLoaderDeferCalls* defer_method_calls = SceneLoader::Get()->GetDeferedCalls();

	defer_method_calls->TryCallDirectly(scene_index, s_remove_box_collider_index, scene_index, entity);
}

void BoxColliderComponentInterface::SetColliderFilter(const CSMonoObject& object, const uint16_t category, const uint16_t mask, const int16_t group_index)
{
	const CSMonoObject game_object = GameObjectInterface::GetGameObjectFromComponent(object);
	const auto scene_index = GameObjectInterface::GetSceneIndex(game_object);
	const auto entity = GameObjectInterface::GetEntityID(game_object);
	BoxColliderComponent& box_collider = SceneManager::GetSceneManager()->GetEntityManager(scene_index)->GetComponent<BoxColliderComponent>(entity);
	box_collider.filter.category_bits = category;
	box_collider.filter.mask_bits = mask;
	box_collider.filter.group_index = group_index;
	box_collider.update_box_collider = true;
}

void BoxColliderComponentInterface::SetTrigger(const CSMonoObject& object, bool trigger)
{
	const CSMonoObject game_object = GameObjectInterface::GetGameObjectFromComponent(object);
	const auto scene_index = GameObjectInterface::GetSceneIndex(game_object);
	const auto entity = GameObjectInterface::GetEntityID(game_object);
	BoxColliderComponent& box_collider = SceneManager::GetSceneManager()->GetEntityManager(scene_index)->GetComponent<BoxColliderComponent>(entity);
	box_collider.trigger = trigger;
	box_collider.update_box_collider = true;
}

void BoxColliderComponentInterface::SetHalfBoxSize(const CSMonoObject& object, const CSMonoObject& half_box_size)
{
	const CSMonoObject game_object = GameObjectInterface::GetGameObjectFromComponent(object);
	const auto scene_index = GameObjectInterface::GetSceneIndex(game_object);
	const auto entity = GameObjectInterface::GetEntityID(game_object);
	BoxColliderComponent& box_collider = SceneManager::GetSceneManager()->GetEntityManager(scene_index)->GetComponent<BoxColliderComponent>(entity);

	box_collider.half_box_size = Vector2Interface::GetVector2(half_box_size);
	box_collider.update_box_collider = true;
}

void BoxColliderComponentInterface::SetOffset(const CSMonoObject& object, const CSMonoObject& offset)
{
	const CSMonoObject game_object = GameObjectInterface::GetGameObjectFromComponent(object);
	const auto scene_index = GameObjectInterface::GetSceneIndex(game_object);
	const auto entity = GameObjectInterface::GetEntityID(game_object);
	BoxColliderComponent& box_collider = SceneManager::GetSceneManager()->GetEntityManager(scene_index)->GetComponent<BoxColliderComponent>(entity);

	box_collider.offset = Vector2Interface::GetVector2(offset);
	box_collider.update_box_collider = true;
}

void BoxColliderComponentInterface::SaveScriptComponent(Entity ent, EntityManager* entman, JsonObject* json_object)
{
	const BoxColliderComponent& box_collider = entman->GetComponent<BoxColliderComponent>(ent);
	json_object->SetData(box_collider.trigger, "trigger");
	json_object->SetData(box_collider.half_box_size, "half_box_size");
	json_object->SetData(box_collider.offset, "offset");
}

void BoxColliderComponentInterface::LoadScriptComponent(Entity ent, EntityManager* entman, JsonObject* json_object)
{
	BoxColliderComponent& box_collider = entman->GetComponent<BoxColliderComponent>(ent);
	Vector2 half_box_size;
	json_object->LoadData(box_collider.trigger, "trigger");
	json_object->LoadData(half_box_size, "half_box_size");
	if (json_object->ObjectExist("offset"))
	{
		json_object->LoadData(box_collider.offset, "offset");
	}
	if (half_box_size.x != 0.0f && half_box_size.y != 0.0f)
		box_collider.half_box_size = half_box_size;
	box_collider.update_box_collider = true;
}
