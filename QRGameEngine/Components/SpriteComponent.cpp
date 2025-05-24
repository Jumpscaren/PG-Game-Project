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
#include "IO/JsonObject.h"
#include "Scripting/Objects/Vector2Interface.h"
#include "Animation/AnimationManager.h"

void SpriteComponentInterface::RegisterInterface(CSMonoCore* mono_core)
{
	auto sprite_class = mono_core->RegisterMonoClass("ScriptProject.Engine", "Sprite");

	mono_core->HookAndRegisterMonoMethodType<SpriteComponentInterface::InitComponent>(sprite_class, "InitComponent", SpriteComponentInterface::InitComponent);
	mono_core->HookAndRegisterMonoMethodType<SpriteComponentInterface::HasComponent>(sprite_class, "HasComponent", SpriteComponentInterface::HasComponent);
	mono_core->HookAndRegisterMonoMethodType<SpriteComponentInterface::RemoveComponent>(sprite_class, "RemoveComponent", SpriteComponentInterface::RemoveComponent);

	mono_core->HookAndRegisterMonoMethodType<SpriteComponentInterface::SetTexture>(sprite_class, "SetTexture", SpriteComponentInterface::SetTexture);
	mono_core->HookAndRegisterMonoMethodType<SpriteComponentInterface::FlipX>(sprite_class, "FlipX", SpriteComponentInterface::FlipX);
	mono_core->HookAndRegisterMonoMethodType<SpriteComponentInterface::FlipY>(sprite_class, "FlipY", SpriteComponentInterface::FlipY);
	mono_core->HookAndRegisterMonoMethodType<SpriteComponentInterface::GetFlipX>(sprite_class, "GetFlipX", SpriteComponentInterface::GetFlipX);
	mono_core->HookAndRegisterMonoMethodType<SpriteComponentInterface::GetFlipY>(sprite_class, "GetFlipY", SpriteComponentInterface::GetFlipY);
	mono_core->HookAndRegisterMonoMethodType<SpriteComponentInterface::SetUV>(sprite_class, "SetUV", SpriteComponentInterface::SetUV);
	mono_core->HookAndRegisterMonoMethodType<SpriteComponentInterface::SetShow>(sprite_class, "SetShow", SpriteComponentInterface::SetShow);

	SceneLoader::Get()->OverrideSaveComponentMethod<SpriteComponent>(SaveSpriteComponent, LoadSpriteComponent);

	AnimationManager::Get()->SetAnimationValue("SpriteComponent", "UV_1", SetUV1);
	AnimationManager::Get()->SetAnimationValue("SpriteComponent", "UV_2", SetUV2);
	AnimationManager::Get()->SetAnimationValue("SpriteComponent", "UV_3", SetUV3);
	AnimationManager::Get()->SetAnimationValue("SpriteComponent", "UV_4", SetUV4);
}

void SpriteComponentInterface::InitComponent(const CSMonoObject& object, SceneIndex scene_index, Entity entity)
{
	SceneManager::GetSceneManager()->GetScene(scene_index)->GetEntityManager()->AddComponent<SpriteComponent>(entity);
}

bool SpriteComponentInterface::HasComponent(const CSMonoObject& object, SceneIndex scene_index, Entity entity)
{
	return SceneManager::GetSceneManager()->GetScene(scene_index)->GetEntityManager()->HasComponent<SpriteComponent>(entity);
}

void SpriteComponentInterface::RemoveComponent(const CSMonoObject& object, SceneIndex scene_index, Entity entity)
{
	SceneManager::GetSceneManager()->GetScene(scene_index)->GetEntityManager()->RemoveComponent<SpriteComponent>(entity);
}

void SpriteComponentInterface::SetTexture(const CSMonoObject& object, const CSMonoObject& texture)
{
	CSMonoObject game_object = ComponentInterface::GetGameObject(object);

	SceneIndex scene_index = GameObjectInterface::GetSceneIndex(game_object);
	Entity entity = GameObjectInterface::GetEntityID(game_object);

	uint64_t texture_handle;
	CSMonoCore::Get()->GetValue(texture_handle, texture, "texture_asset_handle");

	SpriteComponent& sprite_component = SceneManager::GetEntityManager(scene_index)->GetComponent<SpriteComponent>(entity);

	SpriteComponentInterface::LoadTextureToSprite(scene_index, entity, sprite_component, texture_handle);
}

void SpriteComponentInterface::FlipX(const CSMonoObject& object, bool flip_x)
{
	const CSMonoObject game_object = ComponentInterface::GetGameObject(object);

	const SceneIndex scene_index = GameObjectInterface::GetSceneIndex(game_object);
	const Entity entity = GameObjectInterface::GetEntityID(game_object);

	auto& sprite_component = SceneManager::GetEntityManager(scene_index)->GetComponent<SpriteComponent>(entity);

	if (sprite_component.flip_x != flip_x)
	{
		auto& uv_indicies = sprite_component.uv_indicies;
		std::swap(uv_indicies[0], uv_indicies[1]);
		std::swap(uv_indicies[2], uv_indicies[3]);
	}

	sprite_component.flip_x = flip_x;
}

void SpriteComponentInterface::FlipY(const CSMonoObject& object, bool flip_y)
{
	const CSMonoObject game_object = ComponentInterface::GetGameObject(object);

	const SceneIndex scene_index = GameObjectInterface::GetSceneIndex(game_object);
	const Entity entity = GameObjectInterface::GetEntityID(game_object);

	auto& sprite_component = SceneManager::GetEntityManager(scene_index)->GetComponent<SpriteComponent>(entity);

	if (sprite_component.flip_y != flip_y)
	{
		auto& uv_indicies = sprite_component.uv_indicies;
		std::swap(uv_indicies[0], uv_indicies[2]);
		std::swap(uv_indicies[1], uv_indicies[3]);
	}

	sprite_component.flip_y = flip_y;
}

bool SpriteComponentInterface::GetFlipX(const CSMonoObject& object)
{
	const CSMonoObject game_object = ComponentInterface::GetGameObject(object);

	const SceneIndex scene_index = GameObjectInterface::GetSceneIndex(game_object);
	const Entity entity = GameObjectInterface::GetEntityID(game_object);

	return SceneManager::GetEntityManager(scene_index)->GetComponent<SpriteComponent>(entity).flip_x;
}

bool SpriteComponentInterface::GetFlipY(const CSMonoObject& object)
{
	const CSMonoObject game_object = ComponentInterface::GetGameObject(object);

	const SceneIndex scene_index = GameObjectInterface::GetSceneIndex(game_object);
	const Entity entity = GameObjectInterface::GetEntityID(game_object);

	return SceneManager::GetEntityManager(scene_index)->GetComponent<SpriteComponent>(entity).flip_y;
}

void SpriteComponentInterface::SetUV(const CSMonoObject& object, const CSMonoObject& uv_1_position, const CSMonoObject& uv_4_position)
{
	const CSMonoObject game_object = ComponentInterface::GetGameObject(object);

	const SceneIndex scene_index = GameObjectInterface::GetSceneIndex(game_object);
	const Entity entity = GameObjectInterface::GetEntityID(game_object);

	const Vector2 uv_1 = Vector2Interface::GetVector2(uv_1_position);
	const Vector2 uv_4 = Vector2Interface::GetVector2(uv_4_position);

	auto& sprite = SceneManager::GetEntityManager(scene_index)->GetComponent<SpriteComponent>(entity);

	sprite.uv[0] = uv_1;
	sprite.uv[1] = Vector2(uv_4.x, uv_1.y);
	sprite.uv[2] = Vector2(uv_1.x, uv_4.y);
	sprite.uv[3] = uv_4;
}

void SpriteComponentInterface::SetShow(const CSMonoObject& object, const bool show)
{
	const CSMonoObject game_object = ComponentInterface::GetGameObject(object);

	const SceneIndex scene_index = GameObjectInterface::GetSceneIndex(game_object);
	const Entity entity = GameObjectInterface::GetEntityID(game_object);

	SceneManager::GetEntityManager(scene_index)->GetComponent<SpriteComponent>(entity).show = show;
}

void SpriteComponentInterface::SaveSpriteComponent(const Entity ent, EntityManager* entman, JsonObject* json_object)
{
	const SpriteComponent& sprite_component = entman->GetComponent<SpriteComponent>(ent);
	json_object->SetData(sprite_component.texture_handle, "texture_handle");
	json_object->SetData(sprite_component.uv[0], "uv1");
	json_object->SetData(sprite_component.uv[1], "uv2");
	json_object->SetData(sprite_component.uv[2], "uv3");
	json_object->SetData(sprite_component.uv[3], "uv4");
	json_object->SetData(sprite_component.uv_indicies[0], "uv_index_1");
	json_object->SetData(sprite_component.uv_indicies[1], "uv_index_2");
	json_object->SetData(sprite_component.uv_indicies[2], "uv_index_3");
	json_object->SetData(sprite_component.uv_indicies[3], "uv_index_4");
	json_object->SetData(sprite_component.flip_x, "flip_x");
	json_object->SetData(sprite_component.flip_y, "flip_y");
}

void SpriteComponentInterface::LoadSpriteComponent(const Entity ent, EntityManager* entman, JsonObject* json_object)
{
	SpriteComponent& sprite_component = entman->GetComponent<SpriteComponent>(ent);
	json_object->LoadData(sprite_component.texture_handle, "texture_handle"); 

	if (json_object->ObjectExist("uv1"))
	{
		json_object->LoadData(sprite_component.uv[0], "uv1");
		json_object->LoadData(sprite_component.uv[1], "uv2");
		json_object->LoadData(sprite_component.uv[2], "uv3");
		json_object->LoadData(sprite_component.uv[3], "uv4");
	}
	if (json_object->ObjectExist("uv_index_1"))
	{
		json_object->LoadData(sprite_component.uv_indicies[0], "uv_index_1");
		json_object->LoadData(sprite_component.uv_indicies[1], "uv_index_2");
		json_object->LoadData(sprite_component.uv_indicies[2], "uv_index_3");
		json_object->LoadData(sprite_component.uv_indicies[3], "uv_index_4");
	}
	json_object->LoadData(sprite_component.flip_x, "flip_x");
	json_object->LoadData(sprite_component.flip_y, "flip_y");

	if (!SceneLoader::Get()->HasRenderTexture(sprite_component.texture_handle))
		return;

	sprite_component.texture_handle = SceneLoader::Get()->GetRenderTexture(sprite_component.texture_handle);
}

void SpriteComponentInterface::LoadTextureToSprite(SceneIndex scene_index, Entity entity, SpriteComponent& sprite_component, TextureHandle texture_handle)
{
	if (RenderCore::Get()->IsTextureAvailable(sprite_component.texture_handle) && !RenderCore::Get()->IsTextureLoaded(texture_handle))
	{
		RenderCore::Get()->SubscribeEntityToTextureLoading(texture_handle, scene_index, entity);
		return;
	}

	sprite_component.texture_handle = texture_handle;
}

void SpriteComponentInterface::SetUV1(Entity entity, SceneIndex scene_index, Vector2 uv_1)
{
	EntityManager* entity_manager = SceneManager::GetEntityManager(scene_index);
	assert(entity_manager);

	SpriteComponent& sprite = entity_manager->GetComponent<SpriteComponent>(entity);
	sprite.uv[0] = uv_1;
}

void SpriteComponentInterface::SetUV2(Entity entity, SceneIndex scene_index, Vector2 uv_2)
{
	EntityManager* entity_manager = SceneManager::GetEntityManager(scene_index);
	assert(entity_manager);

	SpriteComponent& sprite = entity_manager->GetComponent<SpriteComponent>(entity);
	sprite.uv[1] = uv_2;
}

void SpriteComponentInterface::SetUV3(Entity entity, SceneIndex scene_index, Vector2 uv_3)
{
	EntityManager* entity_manager = SceneManager::GetEntityManager(scene_index);
	assert(entity_manager);

	SpriteComponent& sprite = entity_manager->GetComponent<SpriteComponent>(entity);
	sprite.uv[2] = uv_3;
}

void SpriteComponentInterface::SetUV4(Entity entity, SceneIndex scene_index, Vector2 uv_4)
{
	EntityManager* entity_manager = SceneManager::GetEntityManager(scene_index);
	assert(entity_manager);

	SpriteComponent& sprite = entity_manager->GetComponent<SpriteComponent>(entity);
	sprite.uv[3] = uv_4;
}
