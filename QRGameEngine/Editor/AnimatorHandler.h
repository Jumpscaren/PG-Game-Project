#pragma once
#include "ECS/EntityDefinition.h"
#include "Common/EngineTypes.h"

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
	};

public:
	AnimatorHandler(DrawScene* draw_scene);

	bool AnimationTool();

private:
	AnimationUIData AnimationUI();
	void EditValueFromComponent();

private:
	DrawScene* m_draw_scene = nullptr;

	const std::string m_animation_temp_save_file_name = "animation_temp_save_file_name";
	Entity m_animation_base_entity = NULL_ENTITY;
	std::string m_animation_texture_name;
	std::string m_animation_file_name;

	std::string m_component_name;
	std::string m_edit_component_name;
	std::string m_value_from_component_name;

	struct ValueInAnimation
	{
		std::string component_name;
		std::string value_name;
	};
	std::vector<ValueInAnimation> m_values_used_in_animation;
};