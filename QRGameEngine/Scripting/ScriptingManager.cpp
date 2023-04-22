#include "pch.h"
#include "ScriptingManager.h"
#include "ECS/EntityManager.h"
#include "Components/ScriptComponent.h"
#include "Scripting/CSMonoCore.h"

ScriptingManager* ScriptingManager::s_scripting_manager = nullptr;

ScriptingManager::ScriptingManager()
{
	s_scripting_manager = this;
}

void ScriptingManager::UpdateScripts(EntityManager* entity_manager)
{
	CSMonoCore* mono_core = CSMonoCore::Get();
	entity_manager->System<ScriptComponent>([&](ScriptComponent script)
		{
			if (mono_core->CheckIfMonoMethodExists(script.script_update))
				mono_core->CallMethod(script.script_update, script.script_object);
		});
}

ScriptingManager* ScriptingManager::Get()
{
	return s_scripting_manager;
}
