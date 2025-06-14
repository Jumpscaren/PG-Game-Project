#pragma once
#include "pch.h"
#include "Common/EngineTypes.h"
#include "ECS/EntityDefinition.h"
#include "Components/SpriteComponent.h"

using AnimationSetterId = uint32_t;
using AnimationValueDataId = uint32_t;

enum class AnimationValueType
{
	Float,
	Int,
	Bool,
	Vector2,
	Vector3,
	None,
};

enum class AnimationValueInterpolation
{
	Linear,
	Step,
};

using AnimationKeyFrameId = std::size_t;

struct AnimationKeyFrame
{
	float timestamp;
	AnimationValueInterpolation value_interpolation;
	AnimationValueDataId value_data_id;
};

struct AnimationValueSection
{
	AnimationSetterId value_setter_storage_id;
	AnimationValueType value_type;

	std::vector<AnimationKeyFrame> animation_key_frames;
};

template <typename T>
using AnimationValueSetter = void (*)(Entity, SceneIndex, T);

struct ValueStorage
{
	std::vector<float> animation_value_float_storage;
	std::vector<bool> animation_value_bool_storage;
	std::vector<int> animation_value_int_storage;
	std::vector<Vector2> animation_value_vector2_storage;
	std::vector<Vector3> animation_value_vector3_storage;
};

struct AnimationValueSetterStorage
{
	std::string component_name;
	std::string value_name;
	void* value_setter;
	AnimationValueType value_type;
};

struct ParentAnimationData
{
	std::vector<AnimationValueSection> animation_value_sections;
	std::vector<Entity> children;
	SpriteComponent sprite;
	std::string texture_path;
};
using ParentAnimationDataMap = qr::unordered_map<Entity, ParentAnimationData>;