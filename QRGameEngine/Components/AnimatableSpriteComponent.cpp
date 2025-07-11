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
#include "Renderer/RenderTypes.h"

void AnimatableSpriteComponentInterface::RegisterInterface(CSMonoCore* mono_core)
{
	const auto animatable_sprite_class = mono_core->RegisterMonoClass("ScriptProject.Engine", "AnimatableSprite");

	mono_core->HookAndRegisterMonoMethodType<AnimatableSpriteComponentInterface::InitComponent>(animatable_sprite_class, "InitComponent", AnimatableSpriteComponentInterface::InitComponent);
	mono_core->HookAndRegisterMonoMethodType<AnimatableSpriteComponentInterface::HasComponent>(animatable_sprite_class, "HasComponent", AnimatableSpriteComponentInterface::HasComponent);
	mono_core->HookAndRegisterMonoMethodType<AnimatableSpriteComponentInterface::RemoveComponent>(animatable_sprite_class, "RemoveComponent", AnimatableSpriteComponentInterface::RemoveComponent);

	mono_core->HookAndRegisterMonoMethodType<AnimatableSpriteComponentInterface::SetLoop>(animatable_sprite_class, "SetLoop", AnimatableSpriteComponentInterface::SetLoop);
	mono_core->HookAndRegisterMonoMethodType<AnimatableSpriteComponentInterface::SetId>(animatable_sprite_class, "SetId", AnimatableSpriteComponentInterface::SetId);
	mono_core->HookAndRegisterMonoMethodType<AnimatableSpriteComponentInterface::ResetAnimation>(animatable_sprite_class, "ResetAnimation", AnimatableSpriteComponentInterface::ResetAnimation);
	mono_core->HookAndRegisterMonoMethodType<AnimatableSpriteComponentInterface::SetAnimationSpeed>(animatable_sprite_class, "SetAnimationSpeed", AnimatableSpriteComponentInterface::SetAnimationSpeed);

	SceneLoader::Get()->OverrideSaveComponentMethod<AnimatableSpriteComponent>(SaveAnimatableSpriteComponent, LoadAnimatableSpriteComponent);
}

void AnimatableSpriteComponentInterface::InitComponent(const CSMonoObject& object, const SceneIndex scene_index, const Entity entity)
{
	SceneManager::GetSceneManager()->GetScene(scene_index)->GetEntityManager()->AddComponent<AnimatableSpriteComponent>(entity);
}

bool AnimatableSpriteComponentInterface::HasComponent(const CSMonoObject& object, SceneIndex scene_index, Entity entity)
{
	return SceneManager::GetSceneManager()->GetScene(scene_index)->GetEntityManager()->HasComponent<AnimatableSpriteComponent>(entity);;
}

void AnimatableSpriteComponentInterface::RemoveComponent(const CSMonoObject& object, SceneIndex scene_index, Entity entity)
{
	SceneManager::GetSceneManager()->GetScene(scene_index)->GetEntityManager()->RemoveComponent<AnimatableSpriteComponent>(entity);
}

void AnimatableSpriteComponentInterface::SetLoop(const CSMonoObject& object, bool loop)
{
	const CSMonoObject game_object = GameObjectInterface::GetGameObjectFromComponent(object);
	const auto scene_index = GameObjectInterface::GetSceneIndex(game_object);
	const auto entity = GameObjectInterface::GetEntityID(game_object);
	AnimatableSpriteComponent& anim_sprite = SceneManager::GetSceneManager()->GetEntityManager(scene_index)->GetComponent<AnimatableSpriteComponent>(entity);
	anim_sprite.loop = loop;
}

void AnimatableSpriteComponentInterface::SetId(const CSMonoObject& object, uint32_t id)
{
	const CSMonoObject game_object = GameObjectInterface::GetGameObjectFromComponent(object);
	const auto scene_index = GameObjectInterface::GetSceneIndex(game_object);
	const auto entity = GameObjectInterface::GetEntityID(game_object);
	AnimatableSpriteComponent& anim_sprite = SceneManager::GetSceneManager()->GetEntityManager(scene_index)->GetComponent<AnimatableSpriteComponent>(entity);
	anim_sprite.id = id;
}

void AnimatableSpriteComponentInterface::ResetAnimation(const CSMonoObject& object)
{
	const CSMonoObject game_object = GameObjectInterface::GetGameObjectFromComponent(object);
	const auto scene_index = GameObjectInterface::GetSceneIndex(game_object);
	const auto entity = GameObjectInterface::GetEntityID(game_object);
	AnimatableSpriteComponent& anim_sprite = SceneManager::GetSceneManager()->GetEntityManager(scene_index)->GetComponent<AnimatableSpriteComponent>(entity);
	anim_sprite.finished = false;
}

bool AnimatableSpriteComponentInterface::IsAnimationPlaying(const CSMonoObject& game_object, const uint32_t id)
{
	const auto scene_index = GameObjectInterface::GetSceneIndex(game_object);
	const auto entity = GameObjectInterface::GetEntityID(game_object);
	const AnimatableSpriteComponent& anim_sprite = SceneManager::GetSceneManager()->GetEntityManager(scene_index)->GetComponent<AnimatableSpriteComponent>(entity);

	if (anim_sprite.id != id)
	{
		return false;
	}

	if (anim_sprite.loop)
		return true;

	return !anim_sprite.finished;
}

void AnimatableSpriteComponentInterface::SetAnimationSpeed(const CSMonoObject& object, const float speed)
{
	const CSMonoObject game_object = GameObjectInterface::GetGameObjectFromComponent(object);
	const auto scene_index = GameObjectInterface::GetSceneIndex(game_object);
	const auto entity = GameObjectInterface::GetEntityID(game_object);
	AnimatableSpriteComponent& anim_sprite = SceneManager::GetSceneManager()->GetEntityManager(scene_index)->GetComponent<AnimatableSpriteComponent>(entity);
	
	anim_sprite.speed = speed;
}

void AnimatableSpriteComponentInterface::SaveAnimatableSpriteComponent(Entity ent, EntityManager* entman, JsonObject* json_object)
{
	const AnimatableSpriteComponent& anim_sprite_component = entman->GetComponent<AnimatableSpriteComponent>(ent);
	json_object->SetData(anim_sprite_component.loop, "loop");
}

void AnimatableSpriteComponentInterface::LoadAnimatableSpriteComponent(Entity ent, EntityManager* entman, JsonObject* json_object)
{
	AnimatableSpriteComponent& anim_sprite_component = entman->GetComponent<AnimatableSpriteComponent>(ent);
	json_object->LoadData(anim_sprite_component.loop, "loop");
	json_object->LoadData(anim_sprite_component.max_animation_time, "AnimationTime");
}
