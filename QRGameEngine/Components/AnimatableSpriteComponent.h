#pragma once
#include "Common/EngineTypes.h"
#include "ECS/EntityDefinition.h"
#include "Scripting/CSMonoObject.h"
#include "Renderer/RenderTypes.h"
#include "SceneSystem/SceneManager.h"

struct AnimatableSpriteComponent
{
	Vector2 split_size;
	uint8_t max_split_index;
	uint8_t current_split_index = 0;
	float time_between_splits;
	float time_since_last_split = 0.0f;
	bool loop = false;
	uint8_t id = 0;
	bool finished = false;
};

class JsonObject;
class EntityManager;

class AnimatableSpriteComponentInterface
{
public:
	static void RegisterInterface(CSMonoCore* mono_core);
	static void InitComponent(CSMonoObject object, SceneIndex scene_index, Entity entity);
	static bool HasComponent(CSMonoObject object, SceneIndex scene_index, Entity entity);
	static void RemoveComponent(CSMonoObject object, SceneIndex scene_index, Entity entity);

public:
	static void SetSplitSize(CSMonoObject object, CSMonoObject split_size);
	static void SetMaxSplits(CSMonoObject object, uint32_t max_splits);
	static void SetTimeBetweenSplits(CSMonoObject object, float time_between_splits);
	static void SetLoop(CSMonoObject object, bool loop);
	static void SetId(CSMonoObject object, uint32_t id);
	static void ResetAnimation(CSMonoObject object);
	static bool IsAnimationPlaying(const CSMonoObject& game_object, uint32_t id);

public:
	static void SaveAnimatableSpriteComponent(Entity ent, EntityManager* entman, JsonObject* json_object);
	static void LoadAnimatableSpriteComponent(Entity ent, EntityManager* entman, JsonObject* json_object);

};

