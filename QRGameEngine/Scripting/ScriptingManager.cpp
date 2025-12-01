#include "pch.h"
#include "ScriptingManager.h"
#include "Scripting/CSMonoCore.h"
#include "Event/EventCore.h"
#include "SceneSystem/SceneManager.h"
#include "Scripting/Objects/GameObjectInterface.h"
#include "ECS/EntityManager.h"
#include "Scripting/Objects/Vector2Interface.h"
#include "Time/Time.h"

ScriptingManager* ScriptingManager::s_scripting_manager = nullptr;

void ScriptingManager::ScriptBeginCollision(Entity entity_1, SceneIndex scene_index_1, Entity entity_2, SceneIndex scene_index_2)
{
	EntityManager* entity_manager_1 = SceneManager::GetSceneManager()->GetEntityManager(scene_index_1);
	EntityManager* entity_manager_2 = SceneManager::GetSceneManager()->GetEntityManager(scene_index_2);

	CSMonoCore* mono_core = CSMonoCore::Get();

	if (entity_manager_1->HasComponent<ScriptComponent>(entity_1))
	{
		const ScriptComponent& script = entity_manager_1->GetComponent<ScriptComponent>(entity_1);

		if (mono_core->CheckIfMonoMethodExists(script.script_begin_collision))
		{
			mono_core->CallMethod(script.script_begin_collision, script.script_object, GameObjectInterface::NewGameObjectWithExistingEntity(entity_2, scene_index_2));
		}
	}

	if (entity_manager_2->HasComponent<ScriptComponent>(entity_2))
	{
		const ScriptComponent& script = entity_manager_2->GetComponent<ScriptComponent>(entity_2);

		if (mono_core->CheckIfMonoMethodExists(script.script_begin_collision))
		{
			mono_core->CallMethod(script.script_begin_collision, script.script_object, GameObjectInterface::NewGameObjectWithExistingEntity(entity_1, scene_index_1));
		}
	}
}

void ScriptingManager::ScriptEndCollision(Entity entity_1, SceneIndex scene_index_1, Entity entity_2, SceneIndex scene_index_2)
{
	EntityManager* entity_manager_1 = SceneManager::GetSceneManager()->GetEntityManager(scene_index_1);
	EntityManager* entity_manager_2 = SceneManager::GetSceneManager()->GetEntityManager(scene_index_2);

	CSMonoCore* mono_core = CSMonoCore::Get();

	if (entity_manager_1->HasComponent<ScriptComponent>(entity_1))
	{
		const ScriptComponent& script = entity_manager_1->GetComponent<ScriptComponent>(entity_1);

		if (mono_core->CheckIfMonoMethodExists(script.script_end_collision))
		{
			mono_core->CallMethod(script.script_end_collision, script.script_object, GameObjectInterface::NewGameObjectWithExistingEntity(entity_2, scene_index_2));
		}
	}

	if (entity_manager_2->HasComponent<ScriptComponent>(entity_2))
	{
		const ScriptComponent& script = entity_manager_2->GetComponent<ScriptComponent>(entity_2);

		if (mono_core->CheckIfMonoMethodExists(script.script_end_collision))
		{
			mono_core->CallMethod(script.script_end_collision, script.script_object, GameObjectInterface::NewGameObjectWithExistingEntity(entity_1, scene_index_1));
		}
	}
}

void ScriptingManager::StartScriptsFromActivatedScene(SceneIndex scene_index)
{
	EntityManager* entity_manager = SceneManager::GetSceneManager()->GetEntityManager(scene_index);

	entity_manager->System<ScriptComponent>([&](ScriptComponent& script_component) {
			ScriptingManager::Get()->StartScript(script_component);
		});
}

ScriptingManager::ScriptingManager()
{
	s_scripting_manager = this;

	EventCore::Get()->ListenToEvent<ScriptingManager::ScriptBeginCollision>("BeginCollision", 0, ScriptingManager::ScriptBeginCollision);
	EventCore::Get()->ListenToEvent<ScriptingManager::ScriptEndCollision>("EndCollision", 0, ScriptingManager::ScriptEndCollision);
	EventCore::Get()->ListenToEvent<ScriptingManager::StartScriptsFromActivatedScene>("SceneActivated", 1, ScriptingManager::StartScriptsFromActivatedScene);
	EventCore::Get()->ListenToEvent<ScriptingManager::SetFixedUpdatesToCall>("CurrentPhysicUpdateTicksCount", 0, ScriptingManager::SetFixedUpdatesToCall);
}

void ScriptingManager::StartScript(ScriptComponent& script)
{
	CSMonoCore* mono_core = CSMonoCore::Get();
	if (!script.already_started && mono_core->CheckIfMonoMethodExists(script.script_start))
	{
		script.already_started = true;
		mono_core->CallMethod(script.script_start, script.script_object);
	}
}

void ScriptingManager::UpdateScripts(EntityManager* entity_manager)
{
	CSMonoCore* mono_core = CSMonoCore::Get();
	entity_manager->System<ScriptComponent>([&](ScriptComponent& script)
		{
			if (mono_core->CheckIfMonoMethodExists(script.script_update))
				mono_core->CallMethod(script.script_update, script.script_object);
		});

	while (m_fixed_updates_to_call != 0)
	{
		entity_manager->System<ScriptComponent>([&](ScriptComponent& script)
			{
				if (mono_core->CheckIfMonoMethodExists(script.script_fixed_update))
					mono_core->CallMethod(script.script_fixed_update, script.script_object);
			});

		--m_fixed_updates_to_call;
	}

	entity_manager->System<ScriptComponent>([&](const ScriptComponent& script)
		{
			if (mono_core->CheckIfMonoMethodExists(script.script_late_update))
				mono_core->CallMethod(script.script_late_update, script.script_object);
		});
}

void ScriptingManager::RemoveDeferredScripts(EntityManager* entity_manager)
{
	entity_manager->System<DeferredEntityDeletion, ScriptComponent>([&](DeferredEntityDeletion, ScriptComponent& script)
		{
			InternalRemoveScript(script);
		});
}

void ScriptingManager::AddScript(const SceneIndex scene_index, const Entity entity)
{
	ScriptComponent& script_component = SceneManager::GetSceneManager()->GetScene(scene_index)->GetEntityManager()->GetComponent<ScriptComponent>(entity);

	//script_component.script_object.test.erase(script_component.script_object.m_gchandle);
	script_component.script_start = CSMonoCore::Get()->TryRegisterMonoMethod(script_component.script_object, "Start");
	script_component.script_update = CSMonoCore::Get()->TryRegisterMonoMethod(script_component.script_object, "Update");
	script_component.script_fixed_update = CSMonoCore::Get()->TryRegisterMonoMethod(script_component.script_object, "FixedUpdate");
	script_component.script_late_update = CSMonoCore::Get()->TryRegisterMonoMethod(script_component.script_object, "LateUpdate");
	script_component.script_begin_collision = CSMonoCore::Get()->TryRegisterMonoMethod(script_component.script_object, "BeginCollision");
	if (script_component.script_begin_collision == CSMonoCore::NULL_METHOD)
	{
		const auto parent_class = CSMonoCore::Get()->TryGetParentClass(script_component.script_object);
		if (parent_class != CSMonoCore::NULL_CLASS)
		{
			script_component.script_begin_collision = CSMonoCore::Get()->TryRegisterMonoMethod(parent_class, "BeginCollision");
		}
	}
	script_component.script_end_collision = CSMonoCore::Get()->TryRegisterMonoMethod(script_component.script_object, "EndCollision");

	if (const MonoMethodHandle awake_method_handle = CSMonoCore::Get()->TryRegisterMonoMethod(script_component.script_object, "Awake");
		awake_method_handle != CSMonoCore::NULL_METHOD)
	{
		CSMonoCore::Get()->CallMethod(awake_method_handle, script_component.script_object);
	}

#ifndef _EDITOR
	if (SceneManager::GetSceneManager()->GetScene(scene_index)->IsSceneActive())
		ScriptingManager::Get()->StartScript(script_component);
#endif // _EDITOR
}

void ScriptingManager::RemoveScript(const SceneIndex scene_index, const Entity entity)
{
	ScriptComponent& script_component = SceneManager::GetSceneManager()->GetScene(scene_index)->GetEntityManager()->GetComponent<ScriptComponent>(entity);
	InternalRemoveScript(script_component);
	SceneManager::GetSceneManager()->GetScene(scene_index)->GetEntityManager()->RemoveComponent<ScriptComponent>(entity);
}

void ScriptingManager::InternalRemoveScript(ScriptComponent& script)
{
	const auto script_remove = CSMonoCore::Get()->TryRegisterMonoMethod(script.script_object, "Remove");
	CSMonoCore* mono_core = CSMonoCore::Get();
	if (mono_core->CheckIfMonoMethodExists(script_remove))
	{
		mono_core->CallMethod(script_remove, script.script_object);
	}

	script.script_object.RemoveLinkToMono();
	script.script_start = CSMonoCore::NULL_METHOD;
	script.script_update = CSMonoCore::NULL_METHOD;
	script.script_begin_collision = CSMonoCore::NULL_METHOD;
	script.script_end_collision = CSMonoCore::NULL_METHOD;
}

ScriptingManager* ScriptingManager::Get()
{
	return s_scripting_manager;
}

void ScriptingManager::SetFixedUpdatesToCall(const uint32_t fixed_updates_to_call)
{
	ScriptingManager::Get()->m_fixed_updates_to_call = fixed_updates_to_call;
}
