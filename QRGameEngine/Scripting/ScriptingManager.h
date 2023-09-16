#pragma once
#include "Components/ScriptComponent.h"

class ScriptingManager
{
private:
	static ScriptingManager* s_scripting_manager;

public:
	ScriptingManager();
	void StartScript(const ScriptComponent& script);
	void UpdateScripts(EntityManager* entity_manager);
	void RemoveDeferredScripts(EntityManager* entity_manager);

	static ScriptingManager* Get();
};

