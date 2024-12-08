#pragma once
#include "Physics/PhysicDefines.h"
#include "Scripting/CSMonoObject.h"
#include "ECS/EntityDefinition.h"
#include "SceneSystem/SceneDefines.h"
#include "Common/EngineTypes.h"

struct KinematicBodyComponent
{
	PhysicObjectHandle physic_object_handle;
	bool awake = true;
	Vector2 velocity;
	bool fixed_rotation;
	bool enabled = true;
};

class JsonObject;
class EntityManager;

class KinematicBodyComponentInterface
{
public:
	static void RegisterInterface(CSMonoCore* mono_core);
	static void InitComponent(const CSMonoObject& object, SceneIndex scene_index, Entity entity);
	static bool HasComponent(const CSMonoObject& object, SceneIndex scene_index, Entity entity);
	static void RemoveComponent(const CSMonoObject& object, SceneIndex scene_index, Entity entity);

public:
	static void SetVelocity(const CSMonoObject& object, const CSMonoObject& velocity);
	static CSMonoObject GetVelocity(const CSMonoObject& object);
	static void SetFixedRotation(const CSMonoObject& object, bool fixed_rotation);
	static void SetEnabled(const CSMonoObject& object, const bool enabled);

public:
	static void SaveScriptComponent(Entity ent, EntityManager* entman, JsonObject* json_object);
	static void LoadScriptComponent(Entity ent, EntityManager* entman, JsonObject* json_object);
};

