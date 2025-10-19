#pragma once
#include "Physics/PhysicDefines.h"
#include "Scripting/CSMonoObject.h"
#include "ECS/EntityDefinition.h"
#include "SceneSystem/SceneDefines.h"
#include "Common/DeferMethodCallsDefine.h"

struct PureStaticBodyComponent
{
	PhysicObjectHandle physic_object_handle;
};

class JsonObject;
class EntityManager;

class PureStaticBodyComponentInterface
{
public:
	static void RegisterInterface(CSMonoCore* mono_core, const DeferedMethodIndex add_physic_object_index, const DeferedMethodIndex remove_physic_object_index);
	static void InitComponent(const CSMonoObject& object, SceneIndex scene_index, Entity entity);
	static bool HasComponent(const CSMonoObject& object, SceneIndex scene_index, Entity entity);
	static void RemoveComponent(const CSMonoObject& object, SceneIndex scene_index, Entity entity);

public:
	static void SaveScriptComponent(Entity ent, EntityManager* entman, JsonObject* json_object);
	static void LoadScriptComponent(Entity ent, EntityManager* entman, JsonObject* json_object);

private:
	static DeferedMethodIndex s_add_physic_object_index;
	static DeferedMethodIndex s_remove_physic_object_index;
};

