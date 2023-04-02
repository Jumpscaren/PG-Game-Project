#pragma once
#include "Common/EngineTypes.h"
#include "ECS/EntityManager.h"
#include "Scripting/CSMonoObject.h"
#include "Renderer/RenderTypes.h"

struct SpriteComponent
{
	TextureHandle texture_handle;
	Vector2 uv;
};

class SpriteComponentInterface
{
public:
	static void RegisterInterface(CSMonoCore* mono_core);
	static void InitComponent(CSMonoObject object, SceneIndex scene_index, Entity entity);
	
public:
	static void SetTexture(CSMonoObject object, CSMonoObject texture);
};

