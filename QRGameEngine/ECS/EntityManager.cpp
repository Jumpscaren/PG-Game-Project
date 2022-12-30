#include "pch.h"
#include "EntityManager.h"

bool EntityManager::EntityExists(Entity entity)
{
	return (entity < m_max_entities && m_entities[entity] == entity);
}

bool EntityManager::HasComponent(Entity entity, ComponentPool& component_pool)
{
	return component_pool.m_component_pool_entities.find(entity) != component_pool.m_component_pool_entities.end();
}

EntityManager::EntityManager(uint32_t max_entities) : m_max_entities(max_entities)
{
	assert(m_max_entities != NULL_ENTITY);

	m_entities.resize(max_entities);
	m_free_entities.resize(max_entities);

	const uint32_t max_ent = (max_entities - 1);
	for (uint32_t i = max_ent; i >= 0 && i < max_entities; --i)
	{
		m_entities[i] = NULL_ENTITY;
		m_free_entities[i] = max_ent - i;
	}
}

EntityManager::~EntityManager()
{
	for (int i = 0; i < m_component_pools.size(); ++i)
	{
		free(m_component_pools[i].component_pool_data);
	}
}

Entity EntityManager::NewEntity()
{
	Entity entity = m_free_entities.back();
	m_free_entities.pop_back();

	m_entities[entity] = entity;

	return entity;
}

void EntityManager::RemoveEntity(Entity entity)
{
	assert(EntityExists(entity));

	//If this become a critical point, introduce that entities have a list of components on them aswell
	for (int i = 0; i < m_component_pools.size(); ++i)
	{
		ComponentPool& component_pool = m_component_pools[i];

		if (HasComponent(entity, component_pool))
		{
			component_pool.m_component_pool_entities.erase(entity);
		}
	}

	m_entities[entity] = NULL_ENTITY;
	m_free_entities.push_back(entity);
}
