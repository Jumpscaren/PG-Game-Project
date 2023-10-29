#pragma once
#include "Physics/PhysicDefines.h"
#include "Scripting/CSMonoObject.h"
#include "ECS/EntityDefinition.h"
#include "SceneSystem/SceneDefines.h"
#include "Common/EngineTypes.h"

struct DynamicBodyComponent {
	PhysicObjectHandle physic_object_handle;
	bool awake;
	Vector2 velocity;
	bool fixed_rotation;
};

class JsonObject;
class EntityManager;

class DynamicBodyComponentInterface
{
public:
	static void RegisterInterface(CSMonoCore* mono_core);
	static void InitComponent(CSMonoObject object, SceneIndex scene_index, Entity entity);
	static bool HasComponent(CSMonoObject object, SceneIndex scene_index, Entity entity);
	static void RemoveComponent(CSMonoObject object, SceneIndex scene_index, Entity entity);

public:
	static void SetVelocity(CSMonoObject object, CSMonoObject velocity);
	static CSMonoObject GetVelocity(CSMonoObject object);
	static void SetFixedRotation(CSMonoObject object, bool fixed_rotation);

public:
	static void SaveScriptComponent(Entity ent, EntityManager* entman, JsonObject* json_object);
	static void LoadScriptComponent(Entity ent, EntityManager* entman, JsonObject* json_object);
};

