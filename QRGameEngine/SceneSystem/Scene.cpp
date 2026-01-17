#include "pch.h"
#include "Scene.h"
#include "ECS/EntityManager.h"

Scene::Scene(const SceneIndex scene_index) : m_scene_index(scene_index)
{
	const uint32_t max_entities = MAX_ENTITIES_PER_SCENE;

	m_entity_manager = std::make_unique<EntityManager>(max_entities, scene_index);
	m_scene_loaded = false;
	m_scene_active = false;
}

Scene::~Scene()
{
	
}

void Scene::SetSceneName(const std::string& scene_name)
{
	m_scene_name = scene_name;
}

EntityManager* Scene::GetEntityManager()
{
	return m_entity_manager.get();
}

void Scene::SetSceneAsLoaded()
{
	assert(!m_scene_loaded);
	m_scene_loaded = true;
}

bool Scene::IsSceneLoaded()
{
	return m_scene_loaded;
}

void Scene::SetSceneAsActive()
{
	assert(!m_scene_active);
	m_scene_active = true;
}

bool Scene::IsSceneActive()
{
	return m_scene_active;
}

SceneIndex Scene::GetSceneIndex() const
{
	return m_scene_index;
}

const std::string& Scene::GetSceneName() const
{
	return m_scene_name;
}
