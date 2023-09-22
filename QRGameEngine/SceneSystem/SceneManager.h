#pragma once
#include "Scene.h"
#include "SceneSystem/SceneDefines.h"

class SceneLoader;

class SceneManager
{
private:
	std::vector<Scene*> m_scenes;
	SceneIndex m_active_scene;
	std::vector<SceneIndex> m_free_scene_indicies;

	static SceneManager* s_singleton;

public:
	SceneManager();

	SceneIndex CreateScene();
	Scene* GetScene(SceneIndex scene_index);
	void SetSceneAsActiveScene(SceneIndex scene_index);
	void DestroyScene(SceneIndex scene_index);
	static SceneIndex GetActiveSceneIndex();

	static EntityManager* GetEntityManager(SceneIndex scene_index);

	static SceneManager* GetSceneManager();
};

