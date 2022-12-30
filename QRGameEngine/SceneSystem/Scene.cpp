#include "pch.h"
#include "Scene.h"
#include "ECS/EntityManager.h"

Scene::Scene()
{
	const uint32_t max_entities = 1000;

	m_entity_manager = std::make_unique<EntityManager>(max_entities);
}

Scene::~Scene()
{
	
}

EntityManager* Scene::GetEntityManager()
{
	return m_entity_manager.get();
}
