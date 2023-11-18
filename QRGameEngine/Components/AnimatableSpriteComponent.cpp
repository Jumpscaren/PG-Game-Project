#include "pch.h"
#include "AnimatableSpriteComponent.h"
#include "Scripting/CSMonoCore.h"
#include "SceneSystem/SceneManager.h"
#include "Scripting/Objects/GameObjectInterface.h"
#include "ComponentInterface.h"
#include "ECS/EntityManager.h"
#include "IO/Output.h"
#include "Renderer/RenderCore.h"
#include "SceneSystem/SceneLoader.h"
#include "IO/JsonObject.h"
#include "Scripting/Objects/Vector2Interface.h"

void AnimatableSpriteComponentInterface::RegisterInterface(CSMonoCore* mono_core)
{
	const auto animatable_sprite_class = mono_core->RegisterMonoClass("ScriptProject.Engine", "AnimatableSprite");

	mono_core->HookAndRegisterMonoMethodType<AnimatableSpriteComponentInterface::InitComponent>(animatable_sprite_class, "InitComponent", AnimatableSpriteComponentInterface::InitComponent);
	mono_core->HookAndRegisterMonoMethodType<AnimatableSpriteComponentInterface::HasComponent>(animatable_sprite_class, "HasComponent", AnimatableSpriteComponentInterface::HasComponent);
	mono_core->HookAndRegisterMonoMethodType<AnimatableSpriteComponentInterface::RemoveComponent>(animatable_sprite_class, "RemoveComponent", AnimatableSpriteComponentInterface::RemoveComponent);

	mono_core->HookAndRegisterMonoMethodType<AnimatableSpriteComponentInterface::SetSplitSize>(animatable_sprite_class, "SetSplitSize", AnimatableSpriteComponentInterface::SetSplitSize);
	mono_core->HookAndRegisterMonoMethodType<AnimatableSpriteComponentInterface::SetMaxSplits>(animatable_sprite_class, "SetMaxSplits", AnimatableSpriteComponentInterface::SetMaxSplits);
	mono_core->HookAndRegisterMonoMethodType<AnimatableSpriteComponentInterface::SetTimeBetweenSplits>(animatable_sprite_class, "SetTimeBetweenSplits", AnimatableSpriteComponentInterface::SetTimeBetweenSplits);

	SceneLoader::Get()->OverrideSaveComponentMethod<AnimatableSpriteComponent>(SaveAnimatableSpriteComponent, LoadAnimatableSpriteComponent);
}

void AnimatableSpriteComponentInterface::InitComponent(CSMonoObject object, SceneIndex scene_index, Entity entity)
{
	SceneManager::GetSceneManager()->GetScene(scene_index)->GetEntityManager()->AddComponent<AnimatableSpriteComponent>(entity);
}

bool AnimatableSpriteComponentInterface::HasComponent(CSMonoObject object, SceneIndex scene_index, Entity entity)
{
	return SceneManager::GetSceneManager()->GetScene(scene_index)->GetEntityManager()->HasComponent<AnimatableSpriteComponent>(entity);;
}

void AnimatableSpriteComponentInterface::RemoveComponent(CSMonoObject object, SceneIndex scene_index, Entity entity)
{
	SceneManager::GetSceneManager()->GetScene(scene_index)->GetEntityManager()->RemoveComponent<AnimatableSpriteComponent>(entity);
}

void AnimatableSpriteComponentInterface::SetSplitSize(CSMonoObject object, CSMonoObject split_size)
{
	const CSMonoObject game_object = GameObjectInterface::GetGameObjectFromComponent(object);
	const auto scene_index = GameObjectInterface::GetSceneIndex(game_object);
	const auto entity = GameObjectInterface::GetEntityID(game_object);
	AnimatableSpriteComponent& anim_sprite = SceneManager::GetSceneManager()->GetEntityManager(scene_index)->GetComponent<AnimatableSpriteComponent>(entity);
	anim_sprite.split_size = Vector2Interface::GetVector2(split_size);
}

void AnimatableSpriteComponentInterface::SetMaxSplits(CSMonoObject object, uint32_t max_splits)
{
	const CSMonoObject game_object = GameObjectInterface::GetGameObjectFromComponent(object);
	const auto scene_index = GameObjectInterface::GetSceneIndex(game_object);
	const auto entity = GameObjectInterface::GetEntityID(game_object);
	AnimatableSpriteComponent& anim_sprite = SceneManager::GetSceneManager()->GetEntityManager(scene_index)->GetComponent<AnimatableSpriteComponent>(entity);
	anim_sprite.max_split_index = max_splits;
}

void AnimatableSpriteComponentInterface::SetTimeBetweenSplits(CSMonoObject object, float time_between_splits)
{
	const CSMonoObject game_object = GameObjectInterface::GetGameObjectFromComponent(object);
	const auto scene_index = GameObjectInterface::GetSceneIndex(game_object);
	const auto entity = GameObjectInterface::GetEntityID(game_object);
	AnimatableSpriteComponent& anim_sprite = SceneManager::GetSceneManager()->GetEntityManager(scene_index)->GetComponent<AnimatableSpriteComponent>(entity);
	anim_sprite.time_between_splits = time_between_splits;
}

void AnimatableSpriteComponentInterface::SaveAnimatableSpriteComponent(Entity ent, EntityManager* entman, JsonObject* json_object)
{
	const AnimatableSpriteComponent& anim_sprite_component = entman->GetComponent<AnimatableSpriteComponent>(ent);
	json_object->SetData(anim_sprite_component.split_size, "split_size");
	json_object->SetData(anim_sprite_component.time_between_splits, "time_between_splits");
	json_object->SetData(anim_sprite_component.max_split_index, "max_split_index");
}

void AnimatableSpriteComponentInterface::LoadAnimatableSpriteComponent(Entity ent, EntityManager* entman, JsonObject* json_object)
{
	AnimatableSpriteComponent& anim_sprite_component = entman->GetComponent<AnimatableSpriteComponent>(ent);
	json_object->LoadData(anim_sprite_component.split_size, "split_size");
	json_object->LoadData(anim_sprite_component.time_between_splits, "time_between_splits");
	json_object->LoadData(anim_sprite_component.max_split_index, "max_split_index");
}