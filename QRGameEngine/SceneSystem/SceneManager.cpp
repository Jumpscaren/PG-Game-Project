#include "pch.h"
#include "SceneManager.h"
#include "ECS/EntityManager.h"
#include "SceneLoader.h"

SceneManager* SceneManager::s_singleton;

void SceneManager::DestroyDeferredScenes()
{
	for (const SceneIndex& scene_index: m_deferred_scene_deletion)
	{
		delete m_scenes[scene_index];
		m_scenes[scene_index] = nullptr;

		m_free_scene_indicies.push_back(scene_index);
	}
	m_deferred_scene_deletion.clear();
}

SceneManager::SceneManager()
{
	m_active_scene = -1;
	s_singleton = this;
	m_change_scene_index = NULL_SCENE_INDEX;
	m_load_scene = false;
}

SceneIndex SceneManager::CreateScene()
{
	SceneIndex scene_index = 0;

	if (!m_free_scene_indicies.size())
	{
		scene_index = (SceneIndex)m_scenes.size();
		m_scenes.push_back(new Scene(scene_index));
	}
	else
	{
		scene_index = m_free_scene_indicies.back();

		m_free_scene_indicies.pop_back();
		m_scenes[scene_index] = new Scene(scene_index);
	}

	return scene_index;
}

Scene* SceneManager::GetScene(SceneIndex scene_index)
{
	assert(SceneExists(scene_index));

	return m_scenes[scene_index];
}

void SceneManager::ChangeScene(SceneIndex scene_index)
{
	assert(m_change_scene_index == NULL_SCENE_INDEX && "Trying to change scenes multiple times per frame, not illegal but we want to avoid that happening!");
	m_change_scene_index = scene_index;
}

void SceneManager::DestroyScene(SceneIndex scene_index)
{
	GetScene(scene_index)->GetEntityManager()->RemoveAllEntities();
	m_deferred_scene_deletion.push_back(scene_index);
}

SceneIndex SceneManager::LoadScene(const std::string& scene_name)
{
	assert(!m_load_scene && "Trying to load scenes multiple times per frame, not illegal but we want to avoid that happening!");
	m_load_scene = true;
	m_load_scene_name = scene_name;
	m_load_scene_index = CreateScene();
	return m_load_scene_index;
}

void SceneManager::HandleDeferredScenes()
{
	DestroyDeferredScenes();
	if (m_change_scene_index != NULL_SCENE_INDEX)
	{
		m_active_scene = m_change_scene_index;
	}
	m_change_scene_index = NULL_SCENE_INDEX;
	if (m_load_scene)
	{
		SceneLoader::Get()->LoadScene(m_load_scene_name, m_load_scene_index);
	}
	m_load_scene = false;
}

bool SceneManager::SceneExists(SceneIndex scene_index)
{
	return m_scenes[scene_index];
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
