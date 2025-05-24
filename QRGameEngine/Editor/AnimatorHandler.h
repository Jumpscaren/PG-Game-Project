#pragma once
#include "ECS/EntityDefinition.h"
#include "Common/EngineTypes.h"
#include "Animation/AnimationManager.h"

class DrawScene;

class AnimatorHandler
{
private:
	struct AnimationUIData
	{
		bool back_pressed;
		bool save_animation_pressed;
		bool load_animation_pressed;
		Vector2 uv_1;
		Vector2 uv_4;
		bool texture_pressed;
	};

	struct AnimationValueSectionId
	{
		AnimationSetterId setter_id = 0;
		Entity entity = NULL_ENTITY;

		bool operator==(const AnimationValueSectionId& other) const
		{
			return setter_id == other.setter_id && entity == other.entity;
		}
	};
	struct AnimationValueSectionIdHasher
	{
		std::size_t operator()(const AnimationValueSectionId& k) const
		{
			std::size_t res = 17;
			res = res * 31 + std::hash<AnimationSetterId>()(k.setter_id);
			res = res * 31 + std::hash<Entity>()(k.entity);
			return res;
		}
	};

public:
	AnimatorHandler(DrawScene* draw_scene);

	bool AnimationTool();

private:
	AnimationUIData AnimationUI();
	void EditValueFromComponent();
	void ManageKeyFrames();

	void SaveAnimation(const std::string& folder_path, const std::string& animation_file_name);
	void LoadAnimation(const std::string& folder_path, const std::string& animation_file_name);

	void LoadAnimationVersion1(JsonObject& load_animation, const std::string& folder_path);

	void SaveChildData(JsonObject& parent_data, Entity parent, EntityManager* entity_manager);
	void SaveAnimatableSprite(JsonObject& entity_data, Entity entity, EntityManager* entity_manager);

private:
	void ClearAnimationData();

private:
	DrawScene* m_draw_scene = nullptr;

	const std::string m_animation_temp_save_file_name = "animation_temp_save_file_name";
	Entity m_animation_base_entity = NULL_ENTITY;
	std::string m_animation_texture_name;
	std::string m_animation_file_name;

	Entity m_current_edit_entity = NULL_ENTITY;
	uint64_t m_current_child_index = 0;

	std::string m_component_name;
	std::string m_edit_component_name;
	std::string m_value_from_component_name;
	AnimationSetterId m_current_animation_setter_id;

	Vector2 m_split_size;
	int m_max_split_index;
	float m_time_between_splits;

	struct ValueInAnimation
	{
		std::string component_name;
		std::string value_name;
		AnimationSetterId setter_id;
	};
	qr::unordered_map<Entity, std::vector<ValueInAnimation>> m_values_used_in_animation;

	float m_timeline_time = 0.0f;
	bool m_timeline_loop = true;
	bool m_timeline_play = true;
	float m_animation_max_time = 0.0f;
	float m_animation_timeline_zoom = 1.0f;

	int m_key_frame_index = 0;
	float m_key_frame_time = 0.0f;
	float m_value_float = 0.0f;
	bool m_value_bool = false;
	int m_value_int = 0;
	Vector2 m_value_vector2{};
	AnimationValueInterpolation m_value_interpolation = AnimationValueInterpolation::Linear;

	//Data storage
	ValueStorage m_animation_value_storage;

	qr::unordered_map<AnimationValueSectionId, AnimationValueSection, AnimationValueSectionIdHasher> m_animation_value_sections;
};