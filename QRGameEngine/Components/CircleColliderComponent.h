#pragma once
#include "pch.h"
#include "Physics/PhysicDefines.h"
#include "Scripting/CSMonoObject.h"
#include "ECS/EntityDefinition.h"
#include "SceneSystem/SceneDefines.h"

struct CircleColliderComponent {
	PhysicObjectHandle physic_object_handle;
	float circle_radius;
	bool update_circle_collider = false;
	bool trigger;
	ColliderFilter filter;
	bool debug_draw = false;
};

class JsonObject;
class EntityManager;

class CircleColliderComponentInterface
{
public:
	static void RegisterInterface(CSMonoCore* mono_core);
	static void InitComponent(const CSMonoObject& object, SceneIndex scene_index, Entity entity);
	static bool HasComponent(const CSMonoObject& object, SceneIndex scene_index, Entity entity);
	static void RemoveComponent(const CSMonoObject& object, SceneIndex scene_index, Entity entity);

public:
	static void SetTrigger(const CSMonoObject& object, bool trigger);
	static void SetColliderFilter(const CSMonoObject& object, const uint16_t category, const uint16_t mask, const int16_t group_index);
	static void SetRadius(const CSMonoObject& object, float radius);

public:
	static void SaveScriptComponent(Entity ent, EntityManager* entman, JsonObject* file);
	static void LoadScriptComponent(Entity ent, EntityManager* entman, JsonObject* file);
};

