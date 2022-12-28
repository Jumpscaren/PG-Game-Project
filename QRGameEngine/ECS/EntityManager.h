#pragma once
#include "Vendor/Include/StaticTypeInfo/static_type_info.h"

typedef uint32_t Entity;

class EntityManager
{
private:
	std::vector<Entity> m_free_entities;
	std::vector<Entity> m_entities;

	std::unordered_map<uint64_t, uint32_t> m_component_type_to_pool;

	std::vector<void*> m_component_pools;

	static constexpr Entity NULL_ENTITY = -1;

	uint32_t m_max_entities;

private:
	bool EntityExists(Entity entity);

	template <typename Component>
	uint32_t CreateComponentPool();

public:
	EntityManager(uint32_t max_entities);

	Entity NewEntity();
	void RemoveEntity(Entity entity);

	template <typename Component, typename ...Args>
	Component& AddComponent(Entity entity, Args&& ...args);
};

//----------------------------------------------------------------------------------------------------------------------------------

template<typename Component>
inline uint32_t EntityManager::CreateComponentPool()
{
	uint32_t component_pool_index = m_component_pools.size();

	void* component_pool = malloc(sizeof(Component) * m_max_entities);

	m_component_pools.push_back(component_pool);

	return component_pool_index;
}

template<typename Component, typename ...Args>
inline Component& EntityManager::AddComponent(Entity entity, Args&& ...args)
{
	uint64_t component_index = (uint64_t)static_type_info::getTypeIndex<Component>();

	assert(EntityExists(entity));

	auto it = m_component_type_to_pool.find(component_index);

	uint32_t component_pool_index = 0;

	if (it != m_component_type_to_pool.end())
	{
		component_pool_index = it->second;
	}
	else
	{
		component_pool_index = CreateComponentPool<Component>();
		m_component_type_to_pool.insert({ component_index, component_pool_index });
	}

	char* component_pool = (char*)m_component_pools[component_pool_index];

	Component* new_component = new(component_pool + entity * sizeof(Component)) Component(std::forward<Args>(args)...);

	return *new_component;
}