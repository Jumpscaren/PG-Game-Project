#pragma once
#include "pch.h"
#include "Physics/PhysicDefines.h"
#include "Scripting/CSMonoObject.h"
#include "ECS/EntityDefinition.h"
#include "SceneSystem/SceneDefines.h"
#include "Common/EngineTypes.h"

struct BoxColliderComponent
{
	PhysicObjectHandle physic_object_handle;
	Vector2 half_box_size;
	bool update_box_collider = false;
	bool trigger;
	ColliderFilter filter;
	bool debug_draw = false;
};

class JsonObject;
class EntityManager;

class BoxColliderComponentInterface
{
public:
	static void RegisterInterface(CSMonoCore* mono_core);
	static void InitComponent(const CSMonoObject& object, SceneIndex scene_index, Entity entity);
	static bool HasComponent(const CSMonoObject& object, SceneIndex scene_index, Entity entity);
	static void RemoveComponent(const CSMonoObject& object, SceneIndex scene_index, Entity entity);

public:
	static void SetTrigger(const CSMonoObject& object, bool trigger);
	static void SetHalfBoxSize(const CSMonoObject& object, const CSMonoObject& half_box_size);

public:
	static void SaveScriptComponent(Entity ent, EntityManager* entman, JsonObject* json_object);
	static void LoadScriptComponent(Entity ent, EntityManager* entman, JsonObject* json_object);
};
