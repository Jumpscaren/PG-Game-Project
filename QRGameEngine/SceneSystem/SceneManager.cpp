#include "pch.h"
#include "SceneManager.h"
#include "ECS/EntityManager.h"
#include "SceneLoader.h"
#include "Event/EventCore.h"

SceneManager* SceneManager::s_singleton;

void SceneManager::DestroyDeferredScenes()
{
	for (const SceneIndex scene_index: m_deferred_scene_deletion)
	{
		EventCore::Get()->SendEvent("DeletedScene", scene_index);

		delete m_scenes[scene_index];
		m_scenes[scene_index] = nullptr;

		m_free_scene_indicies.push_back(scene_index);
	}
	m_deferred_scene_deletion.clear();
}

SceneManager::SceneManager()
{
	m_active_scene = NULL_SCENE_INDEX;
	s_singleton = this;
	m_change_scene_index = NULL_SCENE_INDEX;
	m_load_scene = false;
	m_load_scene_index = NULL_SCENE_INDEX;
	m_switch_scene = false;
}

SceneManager::~SceneManager()
{
	for (const Scene* delete_scene : m_scenes)
	{
		delete delete_scene;
	}

	m_deferred_scene_deletion.clear();
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

void SceneManager::ForceChangeScene(SceneIndex scene_index)
{
	ChangeScene(scene_index);
	m_switch_scene = true;
}

void SceneManager::DestroyScene(SceneIndex scene_index)
{
	if (scene_index == NULL_SCENE_INDEX)
	{
		return;
	}
	m_deferred_scene_deletion.push_back(scene_index);
}

SceneIndex SceneManager::LoadScene(const std::string& scene_name)
{
	assert(!m_load_scene && "Trying to load scenes multiple times per frame, not illegal but we want to avoid that happening!");
	m_load_scene = true;
	m_load_scene_name = scene_name;
	m_load_scene_index = CreateScene();
	m_scenes[m_load_scene_index]->SetSceneName(m_load_scene_name);
	return m_load_scene_index;
}

SceneIndex SceneManager::LoadScene(const SceneIndex scene_index)
{
	assert(!m_load_scene && "Trying to load scenes multiple times per frame, not illegal but we want to avoid that happening!");
	m_load_scene = true;
	m_load_scene_name = GetScene(scene_index)->GetSceneName();
	m_load_scene_index = CreateScene();
	m_scenes[m_load_scene_index]->SetSceneName(m_load_scene_name);
	return m_load_scene_index;
}

void SceneManager::RemoveEntitiesFromDeferredDestroyedScenes()
{
	for (const SceneIndex scene_index : m_deferred_scene_deletion)
	{
		GetScene(scene_index)->GetEntityManager()->RemoveAllEntities();
	}
}

void SceneManager::HandleDeferredScenes()
{
	DestroyDeferredScenes();
	if (m_switch_scene)
	{
		m_switch_scene = false;

		m_active_scene = m_change_scene_index;
		GetScene(m_active_scene)->SetSceneAsLoaded();
		EventCore::Get()->SendEvent("SceneLoaded", m_active_scene);
		m_change_scene_index = NULL_SCENE_INDEX;
	}
	if (m_load_scene)
	{
		//SceneLoader::Get()->LoadScene(m_load_scene_name, m_load_scene_index, false);
		SceneLoader::Get()->LoadSceneThreaded(m_load_scene_name, m_load_scene_index);
		//Sleep(10000);
	}
	if (SceneLoader::Get()->FinishedLoadingScene() && m_change_scene_index != NULL_SCENE_INDEX)
	{
		DestroyScene(GetActiveSceneIndex());
		m_switch_scene = true;
	}
	m_load_scene = false;
}

bool SceneManager::SceneExists(SceneIndex scene_index)
{
	return m_scenes[scene_index];
}

bool SceneManager::AlreadyLoadingScene() const
{
	return  m_change_scene_index != NULL_SCENE_INDEX;
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
