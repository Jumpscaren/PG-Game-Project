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
	static void RestartActiveScene();
	static CSMonoObject GetGlobalScene();
	static CSMonoObject LoadScene(const std::string& scene_name);
	static CSMonoObject LoadSceneSynchronized(const std::string& scene_name);
	static bool IsSceneLoaded(const SceneIndex scene_index);
	static void ChangeScene(const SceneIndex scene_index);
};

