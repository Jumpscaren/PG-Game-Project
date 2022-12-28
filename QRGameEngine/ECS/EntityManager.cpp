#include "pch.h"
#include "EntityManager.h"

bool EntityManager::EntityExists(Entity entity)
{
	return (entity < m_max_entities && m_entities[entity] == entity);
}

EntityManager::EntityManager(uint32_t max_entities) : m_max_entities(max_entities)
{
	m_entities.resize(max_entities);
	m_free_entities.resize(max_entities);

	for (uint32_t i = 0; i < max_entities; ++i)
	{
		m_entities[i] = NULL_ENTITY;
		m_free_entities[i] = i;
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

	m_entities[entity] = NULL_ENTITY;
	m_free_entities.push_back(entity);
}
