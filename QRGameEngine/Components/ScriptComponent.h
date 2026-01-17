#pragma once
#include "Common/EngineTypes.h"
#include "Scripting/CSMonoObject.h"
#include "SceneSystem/SceneDefines.h"
#include "ECS/EntityDefinition.h"
#include "Common/DeferMethodCallsDefine.h"

struct ScriptComponent
{
	CSMonoObject script_object;
	bool already_started = false;
	MonoMethodHandle script_start{};
	MonoMethodHandle script_update{};
	float time_since_last_fixed_update = 0.0f;
	MonoMethodHandle script_fixed_update{};
	MonoMethodHandle script_late_update{};
	MonoMethodHandle script_begin_collision{};
	MonoMethodHandle script_end_collision{};
};

class JsonObject;
class EntityManager;

class ScriptComponentInterface
{
public:
	static void RegisterInterface(CSMonoCore* mono_core, const DeferedMethodIndex add_script_component_index, const DeferedMethodIndex remove_script_component_index);
	static void InitComponent(const CSMonoObject& object, SceneIndex scene_index, Entity entity);
	static bool HasComponent(const CSMonoObject& object, SceneIndex scene_index, Entity entity);
	static void RemoveComponent(const CSMonoObject& object, SceneIndex scene_index, Entity entity);

	static void AddScriptComponent(const std::string& script_class_name, SceneIndex scene_index, Entity entity);

public:
	static void SaveScriptComponent(Entity ent, EntityManager* entman, JsonObject* json_object);
	static void LoadScriptComponent(Entity ent, EntityManager* entman, JsonObject* json_object);
	static void RemoveComponentData(ScriptComponent& script_component);

private:
	static DeferedMethodIndex s_add_script_component_index;
	static DeferedMethodIndex s_remove_script_component_index;
};