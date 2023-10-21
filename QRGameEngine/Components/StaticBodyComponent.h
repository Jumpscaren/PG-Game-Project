#pragma once
#include "Physics/PhysicDefines.h"
#include "Scripting/CSMonoObject.h"
#include "ECS/EntityDefinition.h"
#include "SceneSystem/SceneDefines.h"

struct StaticBodyComponent
{
	PhysicObjectHandle physic_object_handle;
};

class JsonObject;
class EntityManager;

class StaticBodyComponentInterface
{
public:
	static void RegisterInterface(CSMonoCore* mono_core);
	static void InitComponent(CSMonoObject object, SceneIndex scene_index, Entity entity);
	static bool HasComponent(CSMonoObject object, SceneIndex scene_index, Entity entity);
	static void RemoveComponent(CSMonoObject object, SceneIndex scene_index, Entity entity);

public:
	static void SaveScriptComponent(Entity ent, EntityManager* entman, JsonObject* json_object);
	static void LoadScriptComponent(Entity ent, EntityManager* entman, JsonObject* json_object);
};

