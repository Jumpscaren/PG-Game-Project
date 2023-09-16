#include "pch.h"
#include "EntityManager.h"
#include "SceneSystem/SceneManager.h"
#include "SceneSystem/Scene.h"

bool EntityManager::EntityExists(Entity entity)
{
	return (entity < m_max_entities && m_entities[entity] == entity);
}

bool EntityManager::HasComponent(Entity entity, ComponentPool& component_pool)
{
	return component_pool.m_component_pool_entities.find(entity) != component_pool.m_component_pool_entities.end();
}

char* EntityManager::GetComponentPoolDataFromName(Entity entity, const std::string& component_name)
{
	auto it = m_component_name_to_pool.find(component_name);

	assert(it != m_component_name_to_pool.end());

	ComponentPool component_pool = m_component_pools[it->second.component_pool_index];

	assert(HasComponent(entity, component_pool));

	return (char*)component_pool.component_pool_data;
}

void EntityManager::DestroyEntity(Entity entity)
{
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

//Quick solution, might have to be significantly improved
void EntityManager::UpdateEntityListIfPoolHasChanged(ComponentPool* component_pool)
{
	if (component_pool->pool_changed)
	{
		component_pool->pool_changed = false;
		component_pool->list_of_component_pool_entities = std::vector<Entity>(component_pool->m_component_pool_entities.begin(), component_pool->m_component_pool_entities.end());
	}
}

EntityManager::EntityManager(uint32_t max_entities) : m_max_entities(max_entities)
{
	assert(m_max_entities != NULL_ENTITY);

	m_entities.resize(max_entities);
	m_free_entities.resize(max_entities);
	m_component_pools.resize(MAX_COMPONENT_POOLS);
	m_current_component_pool_index = 0;

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
	assert(m_free_entities.size());

	Entity entity = m_free_entities.back();
	m_free_entities.pop_back();

	m_entities[entity] = entity;

	return entity;
}

void EntityManager::RemoveEntity(Entity entity)
{
	assert(EntityExists(entity));
	AddComponent<DeferredEntityDeletion>(entity);
}

void EntityManager::DestroyDeferredEntities()
{
	System<DeferredEntityDeletion>([&](Entity entity, DeferredEntityDeletion&) 
		{
			DestroyEntity(entity);
		});
}

Entity EntityManager::CreateEntity(SceneIndex scene_index)
{
	Entity g = SceneManager::GetSceneManager()->GetScene(scene_index)->GetEntityManager()->NewEntity();
	return g;
}

void EntityManager::SetComponentData(Entity entity, const std::string& component_name, void* component_data)
{
	char* component_pool_data = GetComponentPoolDataFromName(entity, component_name);
	auto it = m_component_name_to_pool.find(component_name);

	memcpy(component_pool_data + entity * it->second.component_size, component_data, it->second.component_size);
}

ComponentData EntityManager::GetComponentData(Entity entity, const std::string& component_name)
{
	char* component_pool_data = GetComponentPoolDataFromName(entity, component_name);
	auto it = m_component_name_to_pool.find(component_name);

	ComponentData component_data = {};
	component_data.component_data = component_pool_data + entity * it->second.component_size;
	component_data.component_size = it->second.component_size;

	return component_data;
}

std::vector<std::string> EntityManager::GetComponentNameList(Entity entity)
{
	std::vector<std::string> component_names = {};

	//Slow could be improved by having a map from entity to components
	//This however would require more upkeep which is unnessary as this method should not be used during gameplay frequently
	for (uint32_t i = 0; i < m_current_component_pool_index; ++i)
	{
		if (m_component_pools[i].m_component_pool_entities.contains(entity))
			component_names.push_back(m_component_pools[i].component_name);
	}

	return component_names;
}

uint32_t EntityManager::GetComponentSize(const std::string& component_name)
{
	assert(m_component_name_to_pool.contains(component_name));

	return m_component_name_to_pool.find(component_name)->second.component_size;
}
