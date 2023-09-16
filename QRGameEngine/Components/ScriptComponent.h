#pragma once
#include "Common/EngineTypes.h"
#include "ECS/EntityManager.h"
#include "Scripting/CSMonoObject.h"

struct ScriptComponent
{
	CSMonoObject script_object;
	MonoMethodHandle script_start;
	MonoMethodHandle script_update;
};

class OutputFile;
class EntityManager;

class ScriptComponentInterface
{
public:
	static void RegisterInterface(CSMonoCore* mono_core);
	static void InitComponent(CSMonoObject object, SceneIndex scene_index, Entity entity);
	static bool HasComponent(CSMonoObject object, SceneIndex scene_index, Entity entity);
	static void RemoveComponent(CSMonoObject object, SceneIndex scene_index, Entity entity);

public:
	static void SaveScriptComponent(Entity ent, EntityManager* entman, OutputFile* file);
	static void LoadScriptComponent(Entity ent, EntityManager* entman, OutputFile* file);
	static void RemoveComponentData(ScriptComponent& script_component);
};