#pragma once
#include "pch.h"
#include "Physics/PhysicDefines.h"
#include "Scripting/CSMonoObject.h"
#include "ECS/EntityDefinition.h"
#include "SceneSystem/SceneDefines.h"
#include "Common/DeferMethodCallsDefine.h"

struct CapsuleColliderComponent {
	PhysicObjectHandle physic_object_handle{};
	Vector2 point_1{};
	Vector2 point_2{};
	float radius{};
	bool update_collider = false;
	bool trigger{};
	ColliderFilter filter{};
	bool debug_draw = false;
};

class JsonObject;
class EntityManager;

class CapsuleColliderComponentInterface
{
public:
	static void RegisterInterface(CSMonoCore* mono_core, const DeferedMethodIndex add_physic_object_index, const DeferedMethodIndex add_capsule_collider_index, const DeferedMethodIndex remove_capsule_collider_index);
	static void InitComponent(const CSMonoObject& object, SceneIndex scene_index, Entity entity);
	static bool HasComponent(const CSMonoObject& object, SceneIndex scene_index, Entity entity);
	static void RemoveComponent(const CSMonoObject& object, SceneIndex scene_index, Entity entity);

public:
	static void SetTrigger(const CSMonoObject& object, bool trigger);
	static void SetColliderFilter(const CSMonoObject& object, const uint16_t category, const uint16_t mask, const int16_t group_index);
	static void SetPoints(const CSMonoObject& object, const CSMonoObject& point_1, const CSMonoObject& point_2);
	static void SetRadius(const CSMonoObject& object, float radius);

public:
	static void SaveScriptComponent(Entity ent, EntityManager* entman, JsonObject* file);
	static void LoadScriptComponent(Entity ent, EntityManager* entman, JsonObject* file);

private:
	static DeferedMethodIndex s_add_physic_object_index;
	static DeferedMethodIndex s_add_capsule_collider_index;
	static DeferedMethodIndex s_remove_capsule_collider_index;
};

