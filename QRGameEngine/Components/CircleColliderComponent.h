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
};

class OutputFile;
class EntityManager;

class CircleColliderComponentInterface
{
public:
	static void RegisterInterface(CSMonoCore* mono_core);
	static void InitComponent(CSMonoObject object, SceneIndex scene_index, Entity entity);
	static bool HasComponent(CSMonoObject object, SceneIndex scene_index, Entity entity);
	static void RemoveComponent(CSMonoObject object, SceneIndex scene_index, Entity entity);

public:
	static void SaveScriptComponent(Entity ent, EntityManager* entman, OutputFile* file);
	static void LoadScriptComponent(Entity ent, EntityManager* entman, OutputFile* file);
};

