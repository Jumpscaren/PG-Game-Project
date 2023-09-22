#pragma once
#include "Scripting/CSMonoObject.h"
#include "SceneSystem/SceneDefines.h"

class SceneInterface
{
private:
	static MonoClassHandle scene_object_class;
	static MonoMethodHandle create_scene_with_scene_index;

public:
	static void RegisterInterface(CSMonoCore* mono_core);

public:
	static CSMonoObject CreateSceneWithSceneIndex(SceneIndex scene_index);
};

