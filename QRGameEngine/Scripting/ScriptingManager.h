#pragma once
#include "Components/ScriptComponent.h"
#include "ECS/EntityDefinition.h"
#include "SceneSystem/SceneDefines.h"

class ScriptingManager
{
private:
	static ScriptingManager* s_scripting_manager;

private:
	static void ScriptBeginCollision(Entity entity_1, SceneIndex scene_index_1, Entity entity_2, SceneIndex scene_index_2);
	static void ScriptEndCollision(Entity entity_1, SceneIndex scene_index_1, Entity entity_2, SceneIndex scene_index_2);

public:
	ScriptingManager();
	void StartScript(const ScriptComponent& script);
	void UpdateScripts(EntityManager* entity_manager);
	void RemoveDeferredScripts(EntityManager* entity_manager);

	static ScriptingManager* Get();
};

