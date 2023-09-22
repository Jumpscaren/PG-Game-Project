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
	mono_core->HookAndRegisterMonoMethodType<ScriptComponentInterface::RemoveComponent>(script_class, "RemoveComponent", ScriptComponentInterface::RemoveComponent);

	SceneLoader::Get()->OverrideSaveComponentMethod<ScriptComponent>(SaveScriptComponent, LoadScriptComponent);
}

void ScriptComponentInterface::InitComponent(CSMonoObject object, SceneIndex scene_index, Entity entity)
{
	ScriptComponent& script_component = SceneManager::GetSceneManager()->GetScene(scene_index)->GetEntityManager()->AddComponent<ScriptComponent>(entity);

	script_component.script_object = object;
	script_component.script_start = CSMonoCore::Get()->TryRegisterMonoMethod(object, "Start");
	script_component.script_update = CSMonoCore::Get()->TryRegisterMonoMethod(object, "Update");
	script_component.script_begin_collision = CSMonoCore::Get()->TryRegisterMonoMethod(object, "BeginCollision");
	script_component.script_end_collision = CSMonoCore::Get()->TryRegisterMonoMethod(object, "EndCollision");

	//Change later
	ScriptingManager::Get()->StartScript(script_component);
}

bool ScriptComponentInterface::HasComponent(CSMonoObject object, SceneIndex scene_index, Entity entity)
{
	return SceneManager::GetSceneManager()->GetScene(scene_index)->GetEntityManager()->HasComponent<ScriptComponent>(entity);
}

void ScriptComponentInterface::RemoveComponent(CSMonoObject object, SceneIndex scene_index, Entity entity)
{
	ScriptComponent& script_component = SceneManager::GetSceneManager()->GetScene(scene_index)->GetEntityManager()->GetComponent<ScriptComponent>(entity);

	RemoveComponentData(script_component);

	SceneManager::GetSceneManager()->GetScene(scene_index)->GetEntityManager()->RemoveComponent<ScriptComponent>(entity);
}

void ScriptComponentInterface::AddScriptComponent(const std::string& script_class_name, SceneIndex scene_index, Entity entity)
{
	//Temp
	auto script_class = CSMonoCore::Get()->RegisterMonoClass("ScriptProject", "TestScript");

	CSMonoObject script(CSMonoCore::Get(), script_class);
	InitComponent(script, scene_index, entity);
}

void ScriptComponentInterface::SaveScriptComponent(Entity ent, EntityManager* entman, OutputFile* file)
{
}

void ScriptComponentInterface::LoadScriptComponent(Entity ent, EntityManager* entman, OutputFile* file)
{
}

void ScriptComponentInterface::RemoveComponentData(ScriptComponent& script_component)
{
	script_component.script_object.RemoveLinkToMono();
	script_component.script_start = CSMonoCore::NULL_METHOD;
	script_component.script_update = CSMonoCore::NULL_METHOD;
	script_component.script_begin_collision = CSMonoCore::NULL_METHOD;
	script_component.script_end_collision = CSMonoCore::NULL_METHOD;
}
