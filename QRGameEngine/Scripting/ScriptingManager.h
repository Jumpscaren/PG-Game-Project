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
	static void StartScriptsFromActivatedScene(SceneIndex scene_index);
	void InternalRemoveScript(ScriptComponent& script);

public:
	ScriptingManager();
	void StartScript(ScriptComponent& script);
	void UpdateScripts(EntityManager* entity_manager);
	void RemoveDeferredScripts(EntityManager* entity_manager);

	void AddScript(const SceneIndex scene_index, const Entity entity);
	void RemoveScript(const SceneIndex scene_index, const Entity entity);

	static ScriptingManager* Get();
};

