#include "pch.h"
#include "AnimationManager.h"
#include "IO/JsonObject.h"
#include "IO/OutputFile.h"
#include "ECS/EntityManager.h"
#include "Renderer/RenderCore.h"
#include "Asset/AssetManager.h"
#include "Scripting/CSMonoCore.h"
#include "Scripting/Objects/GameObjectInterface.h"

AnimationManager* AnimationManager::s_singleton = nullptr;

AnimationManager::AnimationManager()
{
	s_singleton = this;
}

AnimationManager* const AnimationManager::Get()
{
	return s_singleton;
}

void AnimationManager::SaveAnimation(const SceneIndex scene_index, const Entity entity, const std::string& animation_file_name)
{
	auto* ent_man = SceneManager::GetSceneManager()->GetEntityManager(scene_index);

	assert(ent_man->HasComponent<AnimatableSpriteComponent>(entity));
	assert(ent_man->HasComponent<SpriteComponent>(entity));

	JsonObject save_animation;
	JsonObject animation_data = save_animation.CreateSubJsonObject("AnimatableSpriteComponent");
	JsonObject sprite_data = save_animation.CreateSubJsonObject("SpriteComponent");

	AnimatableSpriteComponentInterface::SaveAnimatableSpriteComponent(entity, ent_man, &animation_data);
	SpriteComponentInterface::SaveSpriteComponent(entity, ent_man, &sprite_data);
	const SpriteComponent& sprite = ent_man->GetComponent<SpriteComponent>(entity);
	const std::string texture_path = AssetManager::Get()->GetAssetPath(RenderCore::Get()->GetTextureAssetHandle(sprite.texture_handle));

	sprite_data.SetData(texture_path, "Texture_Path");

	OutputFile file(animation_file_name, OutputFile::FileMode::WRITE);
	const std::string json_string = save_animation.GetJsonString();
	file.Write((uint32_t)json_string.size());
	file.Write((void*)json_string.c_str(), (uint32_t)json_string.size());
	file.Close();
}

bool AnimationManager::LoadAnimation(const SceneIndex scene_index, const Entity entity, const std::string& animation_file_name)
{
	auto* ent_man = SceneManager::GetSceneManager()->GetEntityManager(scene_index);

	assert(ent_man->HasComponent<AnimatableSpriteComponent>(entity));
	assert(ent_man->HasComponent<SpriteComponent>(entity));

	AnimatableSpriteComponent& animatable_sprite = ent_man->GetComponent<AnimatableSpriteComponent>(entity);
	SpriteComponent& sprite = ent_man->GetComponent<SpriteComponent>(entity);

	const uint64_t animation_file_name_hash = std::hash<std::string>{}(animation_file_name);
	auto it = m_cached_animation_data.find(animation_file_name_hash);
	if (it != m_cached_animation_data.end())
	{
		animatable_sprite = it->second.animatable_sprite_data;
		sprite = it->second.sprite_data;

		const TextureHandle texture_handle = RenderCore::Get()->LoadTexture(it->second.texture_path, scene_index);
		SpriteComponentInterface::LoadTextureToSprite(scene_index, entity, sprite, texture_handle);

		return true;
	}

	OutputFile file(animation_file_name, OutputFile::FileMode::READ);
	if (!file.FileExists())
	{
		return false;
	}

	const uint32_t json_text_size = file.Read<uint32_t>();
	std::string json_string;
	json_string.resize(json_text_size);
	file.Read((void*)json_string.c_str(), json_text_size);
	file.Close();

	JsonObject load_animation(json_string);
	JsonObject animation_data = load_animation.GetSubJsonObject("AnimatableSpriteComponent");
	JsonObject sprite_data = load_animation.GetSubJsonObject("SpriteComponent");
	AnimatableSpriteComponentInterface::LoadAnimatableSpriteComponent(entity, ent_man, &animation_data);
	SpriteComponentInterface::LoadSpriteComponent(entity, ent_man, &sprite_data);

	std::string sprite_texture_path;
	sprite_data.LoadData(sprite_texture_path, "Texture_Path");

	const TextureHandle texture_handle = RenderCore::Get()->LoadTexture(sprite_texture_path, scene_index);
	SpriteComponentInterface::LoadTextureToSprite(scene_index, entity, sprite, texture_handle);

	animatable_sprite.id = ++m_animation_id;
	AnimationData cached_animation_data;
	cached_animation_data.animatable_sprite_data = animatable_sprite;
	cached_animation_data.sprite_data = sprite;
	cached_animation_data.texture_path = sprite_texture_path;
	m_cached_animation_data.insert({ animation_file_name_hash, cached_animation_data });

	return true;
}

std::string AnimationManager::GetAnimationTexturePath(const SceneIndex scene_index, const Entity entity, const std::string& animation_file_name) const
{
	if (!SceneManager::GetEntityManager(scene_index)->HasComponent<AnimatableSpriteComponent>(entity))
	{
		return "";
	}

	const uint64_t animation_file_name_hash = std::hash<std::string>{}(animation_file_name);
	if (!m_cached_animation_data.contains(animation_file_name_hash))
	{
		return "";

	}

	return m_cached_animation_data.at(animation_file_name_hash).texture_path;
}

void AnimationManager::RegisterInterface(CSMonoCore* mono_core)
{
	const auto animation_manager_class = mono_core->RegisterMonoClass("ScriptProject.Engine", "AnimationManager");

	mono_core->HookAndRegisterMonoMethodType<AnimationManager::SaveAnimationMono>(animation_manager_class, "SaveAnimation", AnimationManager::SaveAnimationMono);
	mono_core->HookAndRegisterMonoMethodType<AnimationManager::LoadAnimationMono>(animation_manager_class, "LoadAnimation", AnimationManager::LoadAnimationMono);
	mono_core->HookAndRegisterMonoMethodType<AnimationManager::IsAnimationPlaying>(animation_manager_class, "IsAnimationPlaying", AnimationManager::IsAnimationPlaying);
}

void AnimationManager::SaveAnimationMono(const CSMonoObject& game_object, const std::string& animation_file_name)
{
	const auto entity = GameObjectInterface::GetEntityID(game_object);
	const auto scene_index = GameObjectInterface::GetSceneIndex(game_object);
	AnimationManager::Get()->SaveAnimation(scene_index, entity, animation_file_name);
}

void AnimationManager::LoadAnimationMono(const CSMonoObject& game_object, const std::string& animation_file_name)
{
	const auto entity = GameObjectInterface::GetEntityID(game_object);
	const auto scene_index = GameObjectInterface::GetSceneIndex(game_object);
	AnimationManager::Get()->LoadAnimation(scene_index, entity, animation_file_name);
}

bool AnimationManager::IsAnimationPlaying(const CSMonoObject& game_object, const std::string& animation_file_name)
{
	const uint64_t animation_file_name_hash = std::hash<std::string>{}(animation_file_name);
	const AnimationManager* anim_manager = AnimationManager::Get();
	const auto it = anim_manager->m_cached_animation_data.find(animation_file_name_hash);
	if (it == anim_manager->m_cached_animation_data.end())
	{
		return false;
	}

	return AnimatableSpriteComponentInterface::IsAnimationPlaying(game_object, it->second.animatable_sprite_data.id);
}
