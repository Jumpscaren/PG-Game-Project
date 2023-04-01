#include "pch.h"
#include "SceneManager.h"

SceneManager* SceneManager::s_singleton;

SceneManager::SceneManager()
{
	m_active_scene = -1;
	s_singleton = this;
}

SceneIndex SceneManager::CreateScene()
{
	SceneIndex scene_index = 0;

	if (!m_free_scene_indicies.size())
	{
		scene_index = (SceneIndex)m_scenes.size();
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

SceneIndex SceneManager::GetActiveSceneIndex()
{
	return s_singleton->m_active_scene;
}

EntityManager* SceneManager::GetEntityManager(SceneIndex scene_index)
{
	return s_singleton->GetScene(scene_index)->GetEntityManager();
}

SceneManager* SceneManager::GetSceneManager()
{
	return s_singleton;
}
