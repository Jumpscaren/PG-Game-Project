#pragma once
#include "Physics/PhysicDefines.h"
#include "Scripting/CSMonoObject.h"
#include "ECS/EntityDefinition.h"
#include "SceneSystem/SceneDefines.h"
#include "Common/EngineTypes.h"
#include "Common/DeferMethodCallsDefine.h"

struct DynamicBodyComponent {
	PhysicObjectHandle physic_object_handle{};
	bool awake = true;
	Vector2 velocity{};
	bool fixed_rotation{};
	bool enabled = true;
};

class JsonObject;
class EntityManager;

class DynamicBodyComponentInterface
{
public:
	static void RegisterInterface(CSMonoCore* mono_core, const DeferedMethodIndex add_physic_object_index, const DeferedMethodIndex add_remove_physic_object_index);
	static void InitComponent(const CSMonoObject& object, SceneIndex scene_index, Entity entity);
	static bool HasComponent(const CSMonoObject& object, SceneIndex scene_index, Entity entity);
	static void RemoveComponent(const CSMonoObject& object, SceneIndex scene_index, Entity entity);

public:
	static void SetVelocity(SceneIndex scene_index, Entity entity, const CSMonoObject& velocity);
	static CSMonoObject GetVelocity(SceneIndex scene_index, Entity entity);
	static void SetFixedRotation(const CSMonoObject& object, bool fixed_rotation);
	static void SetEnabled(const CSMonoObject& object, const bool enabled);

public:
	static void SaveScriptComponent(Entity ent, EntityManager* entman, JsonObject* json_object);
	static void LoadScriptComponent(Entity ent, EntityManager* entman, JsonObject* json_object);

private:
	static DeferedMethodIndex s_add_physic_object_index;
	static DeferedMethodIndex s_remove_physic_object_index;
};

