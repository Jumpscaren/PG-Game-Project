#include "pch.h"
#include "Scene.h"
#include "ECS/EntityManager.h"

Scene::Scene(SceneIndex scene_index) : m_scene_index(scene_index)
{
	const uint32_t max_entities = 10000;

	m_entity_manager = std::make_unique<EntityManager>(max_entities, scene_index);
}

Scene::~Scene()
{
	
}

EntityManager* Scene::GetEntityManager()
{
	return m_entity_manager.get();
}
