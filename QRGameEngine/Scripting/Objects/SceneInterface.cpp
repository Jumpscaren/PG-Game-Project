#include "pch.h"
#include "SceneInterface.h"
#include "Scripting/CSMonoCore.h"

MonoClassHandle SceneInterface::scene_object_class;
MonoMethodHandle SceneInterface::create_scene_with_scene_index;

void SceneInterface::RegisterInterface(CSMonoCore* mono_core)
{
    scene_object_class = mono_core->RegisterMonoClass("ScriptProject.Engine", "Scene");
    create_scene_with_scene_index = mono_core->RegisterMonoMethod(scene_object_class, "CreateSceneWithSceneIndex");
}

CSMonoObject SceneInterface::CreateSceneWithSceneIndex(SceneIndex scene_index)
{
    CSMonoObject scene_object;
    CSMonoCore::Get()->CallStaticMethod(scene_object, create_scene_with_scene_index, scene_index);
    return scene_object;
}
