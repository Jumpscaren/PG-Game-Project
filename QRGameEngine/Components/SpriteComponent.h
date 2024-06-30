#pragma once
#include "Common/EngineTypes.h"
#include "ECS/EntityDefinition.h"
#include "Scripting/CSMonoObject.h"
#include "Renderer/RenderTypes.h"
#include "SceneSystem/SceneManager.h"

struct SpriteComponent
{
	TextureHandle texture_handle;
	Vector2 uv[4] = {
		Vector2(0.0f, 0.0f),
		Vector2(1.0f, 0.0f),
		Vector2(0.0f, 1.0f),
		Vector2(1.0f, 1.0f)};
	uint8_t uv_indicies[4] = { 0, 1, 2, 3 };
	bool flip_x = false;
	bool flip_y = false;
	bool show = true;
};

class JsonObject;
class EntityManager;

class SpriteComponentInterface
{
public:
	static void RegisterInterface(CSMonoCore* mono_core);
	static void InitComponent(const CSMonoObject& object, SceneIndex scene_index, Entity entity);
	static bool HasComponent(const CSMonoObject& object, SceneIndex scene_index, Entity entity);
	static void RemoveComponent(const CSMonoObject& object, SceneIndex scene_index, Entity entity);

public:
	static void SaveSpriteComponent(Entity ent, EntityManager* entman, JsonObject* json_object);
	static void LoadSpriteComponent(Entity ent, EntityManager* entman, JsonObject* json_object);

public:
	static void SetTexture(const CSMonoObject& object, const CSMonoObject& texture);
	static void FlipX(const CSMonoObject& object, bool flip_x);
	static void FlipY(const CSMonoObject& object, bool flip_y);
	static bool GetFlipX(const CSMonoObject& object);
	static bool GetFlipY(const CSMonoObject& object);
	static void SetUV(const CSMonoObject& object, const CSMonoObject& uv_1_position, const CSMonoObject& uv_4_position);
	static void SetShow(const CSMonoObject& object, const bool show);
};

