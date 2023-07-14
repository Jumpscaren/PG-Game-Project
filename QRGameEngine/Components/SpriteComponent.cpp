#include "pch.h"
#include "SpriteComponent.h"
#include "Scripting/CSMonoCore.h"
#include "SceneSystem/SceneManager.h"
#include "Scripting/Objects/GameObjectInterface.h"
#include "ComponentInterface.h"
#include "ECS/EntityManager.h"
#include "IO/Output.h"
#include "Renderer/RenderCore.h"
#include "SceneSystem/SceneLoader.h"

void SpriteComponentInterface::RegisterInterface(CSMonoCore* mono_core)
{
	auto sprite_class = mono_core->RegisterMonoClass("ScriptProject.Engine", "Sprite");

	mono_core->HookAndRegisterMonoMethodType<SpriteComponentInterface::InitComponent>(sprite_class, "InitComponent", SpriteComponentInterface::InitComponent);
	mono_core->HookAndRegisterMonoMethodType<SpriteComponentInterface::HasComponent>(sprite_class, "HasComponent", SpriteComponentInterface::HasComponent);

	mono_core->HookAndRegisterMonoMethodType<SpriteComponentInterface::SetTexture>(sprite_class, "SetTexture", SpriteComponentInterface::SetTexture);

	SceneLoader::Get()->OverrideSaveComponentMethod<SpriteComponent>(SaveSpriteComponent, LoadSpriteComponent);
}

void SpriteComponentInterface::InitComponent(CSMonoObject object, SceneIndex scene_index, Entity entity)
{
	SceneManager::GetSceneManager()->GetScene(scene_index)->GetEntityManager()->AddComponent<SpriteComponent>(entity);
}

bool SpriteComponentInterface::HasComponent(CSMonoObject object, SceneIndex scene_index, Entity entity)
{
	return SceneManager::GetSceneManager()->GetScene(scene_index)->GetEntityManager()->HasComponent<SpriteComponent>(entity);
}

void SpriteComponentInterface::SetTexture(CSMonoObject object, CSMonoObject texture)
{
	CSMonoObject game_object = ComponentInterface::GetGameObject(object);

	SceneIndex scene_index = GameObjectInterface::GetSceneIndex(game_object);
	Entity entity = GameObjectInterface::GetEntityID(game_object);

	uint64_t texture_handle;
	CSMonoCore::Get()->GetValue(texture_handle, texture, "texture_asset_handle");
	
	SceneManager::GetEntityManager(scene_index)->GetComponent<SpriteComponent>(entity).texture_handle = texture_handle;
}

void SpriteComponentInterface::SaveSpriteComponent(Entity ent, EntityManager* entman, OutputFile* file)
{
	SpriteComponent& sprite_component = entman->GetComponent<SpriteComponent>(ent);
	file->Write(sprite_component);
}

void SpriteComponentInterface::LoadSpriteComponent(Entity ent, EntityManager* entman, OutputFile* file)
{
	SpriteComponent& sprite_component = entman->GetComponent<SpriteComponent>(ent);
	sprite_component = file->Read<SpriteComponent>();

	sprite_component.texture_handle = RenderCore::Get()->LoadTexture(SceneLoader::Get()->GetTexturePath(sprite_component.texture_handle));
}
