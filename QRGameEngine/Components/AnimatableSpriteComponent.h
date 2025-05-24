#pragma once
#include "ECS/EntityDefinition.h"
#include "Scripting/CSMonoObject.h"
#include "SceneSystem/SceneDefines.h"
#include "Common/EngineTypes.h"
#include "Animation/AnimationDefines.h"

struct AnimatableSpriteComponent
{
	Vector2 split_size;
	uint8_t max_split_index;
	uint8_t current_split_index = 0;
	float time_between_splits;
	float time_since_last_split = 0.0f;
	float max_animation_time = 0.0f;
	float current_animation_time = 0.0f;
	bool loop = false;
	uint8_t id = 0;
	bool finished = false;
	qr::unordered_map<Entity, std::vector<AnimationKeyFrameId>> key_frames_indicies;
};

class JsonObject;
class EntityManager;

class AnimatableSpriteComponentInterface
{
public:
	static void RegisterInterface(CSMonoCore* mono_core);
	static void InitComponent(const CSMonoObject& object, SceneIndex scene_index, Entity entity);
	static bool HasComponent(const CSMonoObject& object, SceneIndex scene_index, Entity entity);
	static void RemoveComponent(const CSMonoObject& object, SceneIndex scene_index, Entity entity);

public:
	static void SetSplitSize(const CSMonoObject& object, const CSMonoObject& split_size);
	static void SetMaxSplits(const CSMonoObject& object, uint32_t max_splits);
	static void SetTimeBetweenSplits(const CSMonoObject& object, float time_between_splits);
	static void SetLoop(const CSMonoObject& object, bool loop);
	static void SetId(const CSMonoObject& object, uint32_t id);
	static void ResetAnimation(const CSMonoObject& object);
	static bool IsAnimationPlaying(const CSMonoObject& game_object, uint32_t id);

public:
	static void SaveAnimatableSpriteComponent(Entity ent, EntityManager* entman, JsonObject* json_object);
	static void LoadAnimatableSpriteComponent(Entity ent, EntityManager* entman, JsonObject* json_object);
};

