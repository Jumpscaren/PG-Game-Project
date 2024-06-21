#pragma once
#include "Common/EngineTypes.h"
#include "Scripting/CSMonoObject.h"
#include "SceneSystem/SceneDefines.h"
#include "ECS/EntityDefinition.h"

struct ScriptComponent
{
	CSMonoObject script_object;
	MonoMethodHandle script_start;
	MonoMethodHandle script_update;
	MonoMethodHandle script_begin_collision;
	MonoMethodHandle script_end_collision;
};

class JsonObject;
class EntityManager;

class ScriptComponentInterface
{
public:
	static void RegisterInterface(CSMonoCore* mono_core);
	static void InitComponent(const CSMonoObject& object, SceneIndex scene_index, Entity entity);
	static bool HasComponent(const CSMonoObject& object, SceneIndex scene_index, Entity entity);
	static void RemoveComponent(const CSMonoObject& object, SceneIndex scene_index, Entity entity);

	static void AddScriptComponent(const std::string& script_class_name, SceneIndex scene_index, Entity entity);

public:
	static void SaveScriptComponent(Entity ent, EntityManager* entman, JsonObject* json_object);
	static void LoadScriptComponent(Entity ent, EntityManager* entman, JsonObject* json_object);
	static void RemoveComponentData(ScriptComponent& script_component);
};