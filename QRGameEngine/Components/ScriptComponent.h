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

class ScriptComponentInterface
{
public:
	static void RegisterInterface(CSMonoCore* mono_core);
	static void InitComponent(CSMonoObject object, SceneIndex scene_index, Entity entity);
};