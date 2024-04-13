#include "pch.h"
#include "SceneInterface.h"
#include "Scripting/CSMonoCore.h"
#include "SceneSystem/SceneManager.h"

MonoClassHandle SceneInterface::scene_object_class;
MonoMethodHandle SceneInterface::create_scene_with_scene_index;

void SceneInterface::RegisterInterface(CSMonoCore* mono_core)
{
    scene_object_class = mono_core->RegisterMonoClass("ScriptProject.Engine", "Scene");
    create_scene_with_scene_index = mono_core->RegisterMonoMethod(scene_object_class, "CreateSceneWithSceneIndex");

    const auto scene_manager_object_class = mono_core->RegisterMonoClass("ScriptProject.Engine", "SceneManager");
    mono_core->HookAndRegisterMonoMethodType<SceneInterface::RestartActiveScene>(scene_manager_object_class, "RestartActiveScene", SceneInterface::RestartActiveScene);
}

CSMonoObject SceneInterface::CreateSceneWithSceneIndex(SceneIndex scene_index)
{
    CSMonoObject scene_object;
    CSMonoCore::Get()->CallStaticMethod(scene_object, create_scene_with_scene_index, scene_index);
    return scene_object;
}

void SceneInterface::RestartActiveScene()
{
    SceneManager* scene_manager = SceneManager::GetSceneManager();
    const auto scene_index = scene_manager->GetActiveSceneIndex();
    scene_manager->DestroyScene(scene_index);
    SceneIndex scene = scene_manager->LoadScene(scene_index);
    scene_manager->ChangeScene(scene);
}
