#include "pch.h"
#include "ScriptComponent.h"
#include "SceneSystem/SceneManager.h"
#include "Scripting/CSMonoCore.h"
#include "Scripting/ScriptingManager.h"
#include "SceneSystem/SceneLoader.h"

void ScriptComponentInterface::RegisterInterface(CSMonoCore* mono_core)
{
	auto script_class = mono_core->RegisterMonoClass("ScriptProject.Engine", "ScriptingBehaviour");

	mono_core->HookAndRegisterMonoMethodType<ScriptComponentInterface::InitComponent>(script_class, "InitComponent", ScriptComponentInterface::InitComponent);
	mono_core->HookAndRegisterMonoMethodType<ScriptComponentInterface::HasComponent>(script_class, "HasComponent", ScriptComponentInterface::HasComponent);

	SceneLoader::Get()->OverrideSaveComponentMethod<ScriptComponent>(SaveScriptComponent, LoadScriptComponent);
}

void ScriptComponentInterface::InitComponent(CSMonoObject object, SceneIndex scene_index, Entity entity)
{
	ScriptComponent& script_component = SceneManager::GetSceneManager()->GetScene(scene_index)->GetEntityManager()->AddComponent<ScriptComponent>(entity);

	script_component.script_object = object;
	script_component.script_start = CSMonoCore::Get()->TryRegisterMonoMethod(object, "Start");
	script_component.script_update = CSMonoCore::Get()->TryRegisterMonoMethod(object, "Update");

	ScriptingManager::Get()->StartScript(script_component);
}

bool ScriptComponentInterface::HasComponent(CSMonoObject object, SceneIndex scene_index, Entity entity)
{
	return SceneManager::GetSceneManager()->GetScene(scene_index)->GetEntityManager()->HasComponent<ScriptComponent>(entity);
}

void ScriptComponentInterface::SaveScriptComponent(Entity ent, EntityManager* entman, OutputFile* file)
{
}

void ScriptComponentInterface::LoadScriptComponent(Entity ent, EntityManager* entman, OutputFile* file)
{
}
