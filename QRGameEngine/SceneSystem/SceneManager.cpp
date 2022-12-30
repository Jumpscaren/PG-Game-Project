#include "pch.h"
#include "SceneManager.h"

SceneManager::SceneManager()
{
	m_active_scene = -1;
}

SceneIndex SceneManager::CreateScene()
{
	SceneIndex scene_index = 0;

	if (!m_free_scene_indicies.size())
	{
		scene_index = m_scenes.size();
		m_scenes.push_back(new Scene());
	}
	else
	{
		scene_index = m_free_scene_indicies.back();

		m_free_scene_indicies.pop_back();
		m_scenes[scene_index] = new Scene();
	}

	return scene_index;
}

Scene* SceneManager::GetScene(SceneIndex scene_index)
{
	assert(m_scenes[scene_index]);

	return m_scenes[scene_index];
}

void SceneManager::SetSceneAsActiveScene(SceneIndex scene_index)
{
	m_active_scene = scene_index;
}

void SceneManager::DestroyScene(SceneIndex scene_index)
{
	delete m_scenes[scene_index];
	m_scenes[scene_index] = nullptr;

	m_free_scene_indicies.push_back(scene_index);
}
