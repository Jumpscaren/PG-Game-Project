#pragma once
#include "ECS/EntityDefinition.h"
#include "SceneSystem/SceneDefines.h"
#include "Components/AnimatableSpriteComponent.h"
#include "Components/SpriteComponent.h"

class CSMonoCore;
class AnimationManager
{
private:
	struct AnimationData
	{
		AnimatableSpriteComponent animatable_sprite_data;
		SpriteComponent sprite_data;
		std::string texture_path;
	};

private:
	static AnimationManager* s_singleton;
	std::unordered_map<uint64_t, AnimationData> m_cached_animation_data;
	uint8_t m_animation_id = 0;

public:
	static void RegisterInterface(CSMonoCore* mono_core);
	static void SaveAnimationMono(const CSMonoObject game_object, const std::string animation_file_name);
	static void LoadAnimationMono(const CSMonoObject game_object, const std::string animation_file_name);
	static bool IsAnimationPlaying(const CSMonoObject game_object, const std::string animation_file_name);

public:
	AnimationManager();

	static AnimationManager* const Get();

	void SaveAnimation(const SceneIndex scene_index, const Entity entity, const std::string& animation_file_name);
	bool LoadAnimation(const SceneIndex scene_index, const Entity entity, const std::string& animation_file_name);
};

