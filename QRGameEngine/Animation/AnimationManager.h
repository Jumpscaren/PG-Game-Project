#pragma once
#include "ECS/EntityDefinition.h"
#include "SceneSystem/SceneDefines.h"
#include "Components/AnimatableSpriteComponent.h"
#include "Components/SpriteComponent.h"
#include "AnimationDefines.h"

class CSMonoCore;
class AnimationManager
{
private:
	struct AnimationData
	{
		AnimatableSpriteComponent animatable_sprite_data;
		SpriteComponent sprite_data;
		std::string texture_path;

		ParentAnimationDataMap parent_animation_data_map;
		Entity old_parent_entity;
	};

private:
	static AnimationManager* s_singleton;
	qr::unordered_map<uint64_t, uint64_t> m_animation_name_to_cached_data;
	std::vector<AnimationData> m_cached_animation_data;
	uint8_t m_animation_id = 0;

	std::vector<AnimationValueSetterStorage> m_animation_value_setters;

	//Data storage
	ValueStorage m_animation_value_storage;

public:
	static void RegisterInterface(CSMonoCore* mono_core);
	static void SaveAnimationMono(const CSMonoObject& game_object, const std::string& animation_file_name);
	static void LoadAnimationMono(const CSMonoObject& game_object, const std::string& animation_file_name);
	static bool IsAnimationPlaying(const CSMonoObject& game_object, const std::string& animation_file_name);

private:

	template <typename T>
	static AnimationValueType GetValueType()
	{
		assert(false);
		return AnimationValueType::None;
	}

	template <>
	static AnimationValueType GetValueType<float>() { return AnimationValueType::Float; }
	template <>
	static AnimationValueType GetValueType<int>() { return AnimationValueType::Int; }
	template <>
	static AnimationValueType GetValueType<bool>() { return AnimationValueType::Bool; }
	template <>
	static AnimationValueType GetValueType<Vector2>() { return AnimationValueType::Vector2; }
	template <>
	static AnimationValueType GetValueType<Vector3>() { return AnimationValueType::Vector3; }

public:
	AnimationManager();

	static AnimationManager* const Get();

	void UpdateAnimations(EntityManager* ent_man);

	void SaveAnimation(const SceneIndex scene_index, const Entity entity, const std::string& animation_file_name);
	bool LoadAnimation(const SceneIndex scene_index, const Entity entity, const std::string& animation_file_name);
	std::string GetAnimationTexturePath(const SceneIndex scene_index, const Entity entity, const std::string& animation_file_name) const;

	template <typename T>
	void SetAnimationValue(const std::string& component_name, const std::string& value_name, AnimationValueSetter<T> value_setter)
	{
		if (GetAnimationValueSetterStorageIndex(component_name, value_name) != -1)
		{
			assert(false);
			return;
		}

		m_animation_value_setters.push_back(AnimationValueSetterStorage{.component_name = component_name, .value_name = value_name, .value_setter = value_setter, .value_type = GetValueType<T>()});
	}

	AnimationSetterId GetAnimationValueSetterStorageIndex(const std::string& component_name, const std::string& value_name) const;
	const AnimationValueSetterStorage& GetAnimationValueSetterStorage(AnimationSetterId storage_index) const;
	const std::vector<AnimationValueSetterStorage>& GetAnimationValueSetterStorages() const { return m_animation_value_setters; }

	static void LoadChildData(JsonObject& parent_data, Entity parent, EntityManager* entity_manager, ParentAnimationDataMap& parent_animation_data_map, ValueStorage& storage);
	static std::vector<AnimationValueSection> LoadAnimatableSprite(JsonObject& entity_data, EntityManager* entity_manager, ValueStorage& storage);

public:
	static std::vector<AnimationValueSection> LoadAnimationDataAndGetAnimationValueSections(EntityManager* entity_manager, Entity entity, JsonObject& load_animation, ValueStorage& storage);
	static void SetStepAnimationKeyData(const AnimationValueType value_type, const AnimationValueSetterStorage& value_setter_storage, const ValueStorage& value_storage, const AnimationKeyFrame& key_frame, const SceneIndex scene_index, const Entity entity);
	static void SetAnimationKeyData(AnimationKeyFrameId current_key_frame, const AnimationValueSection& animation_value_section, const ValueStorage& value_storage, const SceneIndex scene_index, const Entity entity, const float current_time);
};

