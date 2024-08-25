#pragma once
#include "pch.h"
#include "Physics/PhysicDefines.h"
#include "Scripting/CSMonoObject.h"
#include "ECS/EntityDefinition.h"
#include "SceneSystem/SceneDefines.h"
#include "Common/EngineTypes.h"

struct PolygonColliderComponent
{
	PhysicObjectHandle physic_object_handle;
	std::vector<Vector2> points;
	bool update_polygon_collider = false;
	bool trigger;
	ColliderFilter filter;

	bool loop = false;
	bool solid = false;
};

struct Triangle
{
	Vector2 prev_point;
	Vector2 point;
	Vector2 next_point;
};

class JsonObject;
class EntityManager;

class PolygonColliderComponentInterface
{
public:
	static void RegisterInterface(CSMonoCore* mono_core);
	static void InitComponent(const CSMonoObject& object, SceneIndex scene_index, Entity entity);
	static bool HasComponent(const CSMonoObject& object, SceneIndex scene_index, Entity entity);
	static void RemoveComponent(const CSMonoObject& object, SceneIndex scene_index, Entity entity);

public:
	static std::vector<Triangle> CreatePolygonTriangulation(Entity ent, EntityManager* entman);

public:
	static void SetTrigger(const CSMonoObject& object, bool trigger);

public:
	static void SaveScriptComponent(Entity ent, EntityManager* entman, JsonObject* json_object);
	static void LoadScriptComponent(Entity ent, EntityManager* entman, JsonObject* json_object);
};