#pragma once
#include "Scene.h"
#include "SceneSystem/SceneDefines.h"

class SceneManager
{
private:
	std::vector<Scene*> m_scenes;
	SceneIndex m_active_scene;
	std::vector<SceneIndex> m_free_scene_indicies;

	static SceneManager* s_singleton;

	std::vector<SceneIndex> m_deferred_scene_deletion;
	SceneIndex m_change_scene_index;
	bool m_switch_scene;
	bool m_load_scene;
	std::string m_load_scene_name;
	SceneIndex m_load_scene_index;

private:
	void DestroyDeferredScenes();

public:
	SceneManager();
	~SceneManager();

	SceneIndex CreateScene();
	Scene* GetScene(SceneIndex scene_index);
	void ChangeScene(SceneIndex scene_index);
	void ForceChangeScene(SceneIndex scene_index);
	void DestroyScene(SceneIndex scene_index);
	SceneIndex LoadScene(const std::string& scene_name);
	SceneIndex LoadScene(SceneIndex scene_index);
	void RemoveEntitiesFromDeferredDestroyedScenes();
	void HandleDeferredScenes();
	bool SceneExists(SceneIndex scene_index);
	bool AlreadyLoadingScene() const;

	static SceneIndex GetActiveSceneIndex();
	static EntityManager* GetEntityManager(SceneIndex scene_index);
	static SceneManager* GetSceneManager();
};

