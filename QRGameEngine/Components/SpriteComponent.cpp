#include "pch.h"
#include "SpriteComponent.h"
#include "Scripting/CSMonoCore.h"
#include "SceneSystem/SceneManager.h"
#include "Scripting/Objects/GameObjectInterface.h"
#include "ComponentInterface.h"

void SpriteComponentInterface::RegisterInterface(CSMonoCore* mono_core)
{
	auto sprite_class = mono_core->RegisterMonoClass("ScriptProject.Engine", "Sprite");

	mono_core->HookAndRegisterMonoMethodType<SpriteComponentInterface::InitComponent>(sprite_class, "InitComponent", SpriteComponentInterface::InitComponent);
	mono_core->HookAndRegisterMonoMethodType<SpriteComponentInterface::SetTexture>(sprite_class, "SetTexture", SpriteComponentInterface::SetTexture);
}

void SpriteComponentInterface::InitComponent(CSMonoObject object, SceneIndex scene_index, Entity entity)
{
	SceneManager::GetSceneManager()->GetScene(scene_index)->GetEntityManager()->AddComponent<SpriteComponent>(entity);
}

void SpriteComponentInterface::SetTexture(CSMonoObject object, TextureHandle handle)
{
	CSMonoObject game_object = ComponentInterface::GetGameObject(object);

	SceneIndex scene_index = GameObjectInterface::GetSceneIndex(game_object);
	Entity entity = GameObjectInterface::GetEntityID(game_object);
	
	SceneManager::GetEntityManager(scene_index)->GetComponent<SpriteComponent>(entity).texture_handle = handle;
}
