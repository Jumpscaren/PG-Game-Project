#pragma once
#include "Common/EngineTypes.h"
#include "ECS/EntityDefinition.h"
#include "Scripting/CSMonoObject.h"
#include "Renderer/RenderTypes.h"
#include "SceneSystem/SceneManager.h"

struct SpriteComponent
{
	TextureHandle texture_handle;
	Vector2 uv;
};

class JsonObject;
class EntityManager;

class SpriteComponentInterface
{
public:
	static void RegisterInterface(CSMonoCore* mono_core);
	static void InitComponent(CSMonoObject object, SceneIndex scene_index, Entity entity);
	static bool HasComponent(CSMonoObject object, SceneIndex scene_index, Entity entity);
	static void RemoveComponent(CSMonoObject object, SceneIndex scene_index, Entity entity);

public:
	static void SetTexture(CSMonoObject object, CSMonoObject texture);
	static void SaveSpriteComponent(Entity ent, EntityManager* entman, JsonObject* json_object);
	static void LoadSpriteComponent(Entity ent, EntityManager* entman, JsonObject* json_object);
};

