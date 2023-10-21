#include "pch.h"
#include "Scene.h"
#include "ECS/EntityManager.h"

Scene::Scene(SceneIndex scene_index) : m_scene_index(scene_index)
{
	const uint32_t max_entities = 10000;

	m_entity_manager = std::make_unique<EntityManager>(max_entities, scene_index);
	m_scene_loaded = false;
}

Scene::~Scene()
{
	
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
