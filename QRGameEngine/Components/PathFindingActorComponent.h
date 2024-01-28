#pragma once
#include "ECS/EntityDefinition.h"
#include "Scripting/CSMonoObject.h"
#include "SceneSystem/SceneDefines.h"

struct PathFindingActorComponent
{
};

class JsonObject;
class EntityManager;

class PathFindingActorComponentInterface
{
public:
	static void RegisterInterface(CSMonoCore* mono_core);
	static void InitComponent(CSMonoObject object, SceneIndex scene_index, Entity entity);
	static bool HasComponent(CSMonoObject object, SceneIndex scene_index, Entity entity);
	static void RemoveComponent(CSMonoObject object, SceneIndex scene_index, Entity entity);

public:
	static void SavePathFindingWorldComponent(Entity ent, EntityManager* entman, JsonObject* json_object);
	static void LoadPathFindingWorldComponent(Entity ent, EntityManager* entman, JsonObject* json_object);
};

