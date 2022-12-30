#pragma once
#include "Scene.h"

typedef uint32_t SceneIndex;

class SceneManager
{
private:
	std::vector<Scene*> m_scenes;
	SceneIndex m_active_scene;
	std::vector<SceneIndex> m_free_scene_indicies;

public:
	SceneManager();

	SceneIndex CreateScene();
	Scene* GetScene(SceneIndex scene_index);
	void SetSceneAsActiveScene(SceneIndex scene_index);
	void DestroyScene(SceneIndex scene_index);
};

