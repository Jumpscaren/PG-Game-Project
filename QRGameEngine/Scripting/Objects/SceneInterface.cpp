#include "pch.h"
#include "SceneInterface.h"
#include "Scripting/CSMonoCore.h"
#include "SceneSystem/SceneManager.h"
#include "SceneSystem/GlobalScene.h"

MonoClassHandle SceneInterface::scene_object_class;
MonoMethodHandle SceneInterface::create_scene_with_scene_index;

void SceneInterface::RegisterInterface(CSMonoCore* mono_core)
{
    scene_object_class = mono_core->RegisterMonoClass("ScriptProject.Engine", "Scene");
    create_scene_with_scene_index = mono_core->RegisterMonoMethod(scene_object_class, "CreateSceneWithSceneIndex");

    const auto scene_manager_object_class = mono_core->RegisterMonoClass("ScriptProject.Engine", "SceneManager");
    mono_core->HookAndRegisterMonoMethodType<SceneInterface::RestartActiveScene>(scene_manager_object_class, "RestartActiveScene", SceneInterface::RestartActiveScene);
    mono_core->HookAndRegisterMonoMethodType<SceneInterface::GetGlobalScene>(scene_manager_object_class, "GetGlobalScene", SceneInterface::GetGlobalScene);
    mono_core->HookAndRegisterMonoMethodType<SceneInterface::LoadScene>(scene_manager_object_class, "LoadScene", SceneInterface::LoadScene);
    mono_core->HookAndRegisterMonoMethodType<SceneInterface::LoadSceneSynchronized>(scene_manager_object_class, "LoadSceneSynchronized", SceneInterface::LoadSceneSynchronized);
    mono_core->HookAndRegisterMonoMethodType<SceneInterface::IsSceneLoaded>(scene_manager_object_class, "IsSceneLoaded_External", SceneInterface::IsSceneLoaded);
    mono_core->HookAndRegisterMonoMethodType<SceneInterface::ChangeScene>(scene_manager_object_class, "ChangeScene_External", SceneInterface::ChangeScene);
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

    if (scene_manager->AlreadyLoadingScene())
    {
        std::cout << "Already loading scene" << std::endl;
        return;
    }

    const auto scene_index = scene_manager->GetActiveSceneIndex();
    //scene_manager->DestroyScene(scene_index);
    SceneIndex scene = scene_manager->LoadScene(scene_index, true);
    scene_manager->ChangeScene(scene);
}

CSMonoObject SceneInterface::GetGlobalScene()
{
    return SceneInterface::CreateSceneWithSceneIndex(GlobalScene::Get()->GetSceneIndex());
}

CSMonoObject SceneInterface::LoadScene(const std::string& scene_name)
{
    return SceneInterface::CreateSceneWithSceneIndex(SceneManager::GetSceneManager()->LoadScene(scene_name, true));
}

CSMonoObject SceneInterface::LoadSceneSynchronized(const std::string& scene_name)
{
    return SceneInterface::CreateSceneWithSceneIndex(SceneManager::GetSceneManager()->LoadScene(scene_name, false));
}

bool SceneInterface::IsSceneLoaded(const SceneIndex scene_index)
{
    return SceneManager::GetSceneManager()->GetScene(scene_index)->IsSceneLoaded();
}

void SceneInterface::ChangeScene(const SceneIndex scene_index)
{
    SceneManager::GetSceneManager()->ChangeScene(scene_index);
}
