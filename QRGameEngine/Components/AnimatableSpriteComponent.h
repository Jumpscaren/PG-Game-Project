#pragma once
#include "ECS/EntityDefinition.h"
#include "Scripting/CSMonoObject.h"
#include "SceneSystem/SceneDefines.h"
#include "Common/EngineTypes.h"
#include "Animation/AnimationDefines.h"

struct AnimatableSpriteComponent
{
	float max_animation_time = 0.0f;
	float current_animation_time = 0.0f;
	bool loop = false;
	uint8_t id = 0;
	bool finished = false;
	qr::unordered_map<Entity, std::vector<AnimationKeyFrameId>> key_frames_indicies;
	float speed = 1.0f;
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
	static void SetLoop(const CSMonoObject& object, bool loop);
	static void SetId(const CSMonoObject& object, uint32_t id);
	static void ResetAnimation(const CSMonoObject& object);
	static bool IsAnimationPlaying(const CSMonoObject& game_object, uint32_t id);
	static void SetAnimationSpeed(const CSMonoObject& game_object, float speed);

public:
	static void SaveAnimatableSpriteComponent(Entity ent, EntityManager* entman, JsonObject* json_object);
	static void LoadAnimatableSpriteComponent(Entity ent, EntityManager* entman, JsonObject* json_object);
};

