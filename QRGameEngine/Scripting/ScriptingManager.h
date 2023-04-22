#pragma once

class EntityManager;

class ScriptingManager
{
private:
	static ScriptingManager* s_scripting_manager;

public:
	ScriptingManager();
	void UpdateScripts(EntityManager* entity_manager);

	static ScriptingManager* Get();
};

