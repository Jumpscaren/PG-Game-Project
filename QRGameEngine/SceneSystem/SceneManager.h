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

	struct SceneLoading {
		std::string load_scene_name;
		SceneIndex load_scene_index;
		bool load_scene_in_thread;
	};

	std::vector<SceneLoading> m_scenes_to_be_loading;
	std::optional<SceneLoading> m_loading_scene;

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
	SceneIndex LoadScene(const std::string& scene_name, bool asynchronized);
	SceneIndex LoadScene(SceneIndex scene_index, bool asynchronized);
	void RemoveEntitiesFromDeferredDestroyedScenes();
	void HandleDeferredScenes();
	bool SceneExists(SceneIndex scene_index);
	bool AlreadyLoadingScene() const;

	static SceneIndex GetActiveSceneIndex();
	static EntityManager* GetEntityManager(SceneIndex scene_index);
	static SceneManager* GetSceneManager();
};

