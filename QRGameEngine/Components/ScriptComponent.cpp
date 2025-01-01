#include "pch.h"
#include "ScriptComponent.h"
#include "SceneSystem/SceneManager.h"
#include "Scripting/CSMonoCore.h"
#include "Scripting/ScriptingManager.h"
#include "SceneSystem/SceneLoader.h"
#include "IO/JsonObject.h"

void ScriptComponentInterface::RegisterInterface(CSMonoCore* mono_core)
{
	auto script_class = mono_core->RegisterMonoClass("ScriptProject.Engine", "ScriptingBehaviour");

	mono_core->HookAndRegisterMonoMethodType<ScriptComponentInterface::InitComponent>(script_class, "InitComponent", ScriptComponentInterface::InitComponent);
	mono_core->HookAndRegisterMonoMethodType<ScriptComponentInterface::HasComponent>(script_class, "HasComponent", ScriptComponentInterface::HasComponent);
	mono_core->HookAndRegisterMonoMethodType<ScriptComponentInterface::RemoveComponent>(script_class, "RemoveComponent", ScriptComponentInterface::RemoveComponent);

	SceneLoader::Get()->OverrideSaveComponentMethod<ScriptComponent>(SaveScriptComponent, LoadScriptComponent);
}

void ScriptComponentInterface::InitComponent(const CSMonoObject& object, SceneIndex scene_index, Entity entity)
{
	ScriptComponent& script_component = SceneManager::GetSceneManager()->GetScene(scene_index)->GetEntityManager()->AddComponent<ScriptComponent>(entity);

	script_component.script_object = object;
	//script_component.script_object.test.erase(script_component.script_object.m_gchandle);
	script_component.script_start = CSMonoCore::Get()->TryRegisterMonoMethod(object, "Start");
	script_component.script_update = CSMonoCore::Get()->TryRegisterMonoMethod(object, "Update");
	script_component.script_begin_collision = CSMonoCore::Get()->TryRegisterMonoMethod(object, "BeginCollision");
	if (script_component.script_begin_collision == CSMonoCore::NULL_METHOD)
	{
		const auto parent_class = CSMonoCore::Get()->TryGetParentClass(object);
		if (parent_class != CSMonoCore::NULL_CLASS)
		{
			script_component.script_begin_collision = CSMonoCore::Get()->TryRegisterMonoMethod(parent_class, "BeginCollision");
		}
	}
	script_component.script_end_collision = CSMonoCore::Get()->TryRegisterMonoMethod(object, "EndCollision");

	if (const MonoMethodHandle awake_method_handle = CSMonoCore::Get()->TryRegisterMonoMethod(object, "Awake"); 
		awake_method_handle != CSMonoCore::NULL_METHOD)
	{
		CSMonoCore::Get()->CallMethod(awake_method_handle, object);
	}

#ifndef _EDITOR
	if (SceneManager::GetSceneManager()->GetScene(scene_index)->IsSceneActive())
		ScriptingManager::Get()->StartScript(script_component);
#endif // _EDITOR
}

bool ScriptComponentInterface::HasComponent(const CSMonoObject& object, SceneIndex scene_index, Entity entity)
{
	return SceneManager::GetSceneManager()->GetScene(scene_index)->GetEntityManager()->HasComponent<ScriptComponent>(entity);
}

void ScriptComponentInterface::RemoveComponent(const CSMonoObject& object, SceneIndex scene_index, Entity entity)
{
	ScriptComponent& script_component = SceneManager::GetSceneManager()->GetScene(scene_index)->GetEntityManager()->GetComponent<ScriptComponent>(entity);

	ScriptingManager::Get()->RemoveScript(script_component);

	SceneManager::GetSceneManager()->GetScene(scene_index)->GetEntityManager()->RemoveComponent<ScriptComponent>(entity);
}

void ScriptComponentInterface::AddScriptComponent(const std::string& script_class_name, SceneIndex scene_index, Entity entity)
{
	auto script_class = CSMonoCore::Get()->RegisterMonoClass("ScriptProject", script_class_name);

	CSMonoObject script(CSMonoCore::Get(), script_class);
	InitComponent(script, scene_index, entity);
}

void ScriptComponentInterface::SaveScriptComponent(Entity ent, EntityManager* entman, JsonObject* json_object)
{
	const ScriptComponent& script_component = entman->GetComponent<ScriptComponent>(ent);
	const std::vector<std::string> field_names = CSMonoCore::Get()->GetAllFieldNames(script_component.script_object);
	for (const std::string& field_name : field_names)
	{
		if (CSMonoCore::Get()->IsValueType<uint32_t>(script_component.script_object, field_name))
		{
			uint32_t value;
			CSMonoCore::Get()->GetValue(value, script_component.script_object, field_name);
			json_object->SetData(value, field_name);
		}
		else if (CSMonoCore::Get()->IsValueType<bool>(script_component.script_object, field_name))
		{
			bool value;
			CSMonoCore::Get()->GetValue(value, script_component.script_object, field_name);
			json_object->SetData(value, field_name);
		}
		else if (CSMonoCore::Get()->IsValueType<double>(script_component.script_object, field_name))
		{
			double value;
			CSMonoCore::Get()->GetValue(value, script_component.script_object, field_name);
			json_object->SetData(value, field_name);
		}
		else if (CSMonoCore::Get()->IsValueType<std::string>(script_component.script_object, field_name))
		{
			std::string value;
			CSMonoCore::Get()->GetValue(value, script_component.script_object, field_name);
			json_object->SetData(value, field_name);
		}
		//else if (CSMonoCore::Get()->IsValueType<CSMonoObject>(script_component.script_object, field_name))
		//{
		//	CSMonoObject value;
		//	//CSMonoCore::Get()->GetValue(value, script_component.script_object, field_name);
		//	//json_object->SetData(value, field_name);
		//}
	}
}

void ScriptComponentInterface::LoadScriptComponent(Entity ent, EntityManager* entman, JsonObject* json_object)
{
	const ScriptComponent& script_component = entman->GetComponent<ScriptComponent>(ent);
	const std::vector<std::string> field_names = CSMonoCore::Get()->GetAllFieldNames(script_component.script_object);
	for (const std::string& field_name : field_names)
	{
		if (CSMonoCore::Get()->IsValueType<uint32_t>(script_component.script_object, field_name))
		{
			uint32_t value;
			json_object->LoadData(value, field_name);
			CSMonoCore::Get()->SetValue(value, script_component.script_object, field_name);
		}
		else if (CSMonoCore::Get()->IsValueType<bool>(script_component.script_object, field_name))
		{
			bool value;
			json_object->LoadData(value, field_name);
			CSMonoCore::Get()->SetValue(value, script_component.script_object, field_name);
		}
		else if (CSMonoCore::Get()->IsValueType<double>(script_component.script_object, field_name))
		{
			double value;
			json_object->LoadData(value, field_name);
			CSMonoCore::Get()->SetValue(value, script_component.script_object, field_name);
		}
		else if (CSMonoCore::Get()->IsValueType<std::string>(script_component.script_object, field_name))
		{
			std::string value;
			json_object->LoadData(value, field_name);
			CSMonoCore::Get()->SetValue(value, script_component.script_object, field_name);
		}
		//else if (CSMonoCore::Get()->IsValueType<CSMonoObject>(script_component.script_object, field_name))
		//{
		//	CSMonoObject value;
		//	//CSMonoCore::Get()->GetValue(value, script_component.script_object, field_name);
		//	//json_object->SetData(value, field_name);
		//}
	}
}

void ScriptComponentInterface::RemoveComponentData(ScriptComponent& script_component)
{

}
