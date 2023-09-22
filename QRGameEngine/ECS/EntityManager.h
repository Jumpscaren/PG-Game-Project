#pragma once
#include "Vendor/Include/StaticTypeInfo/static_type_info.h"
#include "EntityDefinition.h"
#include "SceneSystem/SceneDefines.h"

struct ComponentPool
{
	void* component_pool_data;
	std::unordered_set<Entity> m_component_pool_entities;
	bool pool_changed = false;
	std::vector<Entity> list_of_component_pool_entities;
	std::string component_name;
};

struct ComponentData
{
	char* component_data;
	uint32_t component_size;
};

struct DeferredEntityDeletion
{

};

class EntityManager
{
private:
	struct NameToPoolData
	{
		uint32_t component_pool_index;
		uint32_t component_size;
	};

private:
	SceneIndex m_entity_manager_scene_index;

	std::vector<Entity> m_free_entities;
	std::vector<Entity> m_entities;

	std::unordered_map<uint64_t, uint32_t> m_component_type_to_pool;
	std::unordered_map<std::string, NameToPoolData> m_component_name_to_pool;

	std::vector<ComponentPool> m_component_pools;
	uint32_t m_current_component_pool_index;
	static constexpr uint32_t MAX_COMPONENT_POOLS = 100;

	uint32_t m_max_entities;

private:
	template <typename Component>
	uint32_t CreateComponentPool();

	template <typename Component>
	ComponentPool& GetComponentPool();

	bool HasComponent(Entity entity, ComponentPool& component_pool);

	char* GetComponentPoolDataFromName(Entity entity, const std::string& component_name);

	void DestroyEntity(Entity entity);

	void UpdateEntityListIfPoolHasChanged(ComponentPool* component_pool);

public:
	EntityManager(uint32_t max_entities, SceneIndex scene_index);
	~EntityManager();

	SceneIndex GetSceneIndex() const;

	Entity NewEntity();
	void RemoveEntity(Entity entity);
	void DestroyDeferredEntities();

	static Entity CreateEntity(SceneIndex scene_index);

	bool EntityExists(Entity entity);

	template <typename Component, typename ...Args>
	Component& AddComponent(Entity entity, Args&& ...args);

	template <typename Component>
	bool HasComponent(Entity entity);

	template <typename Component>
	Component& GetComponent(Entity entity);

	template <typename Component>
	void RemoveComponent(Entity entity);

	template<typename Component>
	static std::string GetComponentNameFromComponent();

	void SetComponentData(Entity entity, const std::string& component_name, void* component_data);

	ComponentData GetComponentData(Entity entity, const std::string& component_name);

	std::vector<std::string> GetComponentNameList(Entity entity);

	uint32_t GetComponentSize(const std::string& component_name);

	template <typename... Component>
	void System(std::invocable<Component&...> auto&& func)
	{
		std::vector<ComponentPool*> pools;

		(pools.push_back(&GetComponentPool<Component>()), ...);

		assert(pools.size() != 0);

		ComponentPool* smallest_component_pool = pools[0];
		for (int i = 1; i < pools.size(); ++i)
		{
			if (pools[i]->m_component_pool_entities.size() < smallest_component_pool->m_component_pool_entities.size())
			{
				smallest_component_pool = pools[i];
			}
		}

		UpdateEntityListIfPoolHasChanged(smallest_component_pool);

		auto& entities = smallest_component_pool->list_of_component_pool_entities;
		for (Entity& entity : entities)
		{
			if (!EntityExists(entity))
				continue;

			bool has_all_components = (HasComponent<Component>(entity) &&...);

			if (has_all_components)
			{
				func(GetComponent<Component>(entity)...);
			}
		}
	}

	template <typename... Component>
	void System(std::invocable<Entity ,Component&...> auto&& func)
	{
		std::vector<ComponentPool*> pools;

		(pools.push_back(&GetComponentPool<Component>()), ...);

		assert(pools.size() != 0);

		ComponentPool* smallest_component_pool = pools[0];
		for (int i = 1; i < pools.size(); ++i)
		{
			if (pools[i]->m_component_pool_entities.size() < smallest_component_pool->m_component_pool_entities.size())
			{
				smallest_component_pool = pools[i];
			}
		}

		UpdateEntityListIfPoolHasChanged(smallest_component_pool);

		auto& entities = smallest_component_pool->list_of_component_pool_entities;
		for (Entity& entity : entities)
		{
			if (!EntityExists(entity))
				continue;

			bool has_all_components = (HasComponent<Component>(entity) &&...);

			if (has_all_components)
			{
				func(entity, GetComponent<Component>(entity)...);
			}
		}
	}
};

//----------------------------------------------------------------------------------------------------------------------------------

template<typename Component>
inline uint32_t EntityManager::CreateComponentPool()
{
	//uint32_t component_pool_index = (uint32_t)m_component_pools.size(); 
	uint32_t component_pool_index = m_current_component_pool_index++;

	assert(component_pool_index < MAX_COMPONENT_POOLS);

	void* component_pool_data = malloc(sizeof(Component) * m_max_entities);

	ComponentPool component_pool;
	component_pool.component_pool_data = component_pool_data;
	component_pool.component_name = GetComponentNameFromComponent<Component>();

	//m_component_pools.push_back(component_pool);
	m_component_pools[component_pool_index] = component_pool;

	m_component_name_to_pool.insert({ component_pool.component_name, {component_pool_index, (uint32_t)sizeof(Component)}});

	return component_pool_index;
}

template<typename Component>
inline ComponentPool& EntityManager::GetComponentPool()
{
	uint64_t component_index = (uint64_t)static_type_info::getTypeIndex<Component>();

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

	return m_component_pools[component_pool_index];
}

template<typename Component>
inline std::string EntityManager::GetComponentNameFromComponent()
{
	std::string component_name = typeid(Component).name();
	//Removes the "struct " from the "struct componentname" which comes from typeid(Component).name()
	component_name.erase(0, 7);

	return component_name;
}

template<typename Component, typename ...Args>
inline Component& EntityManager::AddComponent(Entity entity, Args&& ...args)
{
	assert(EntityExists(entity));

	ComponentPool& component_pool = GetComponentPool<Component>();

	assert(!HasComponent(entity, component_pool));

	component_pool.m_component_pool_entities.insert(entity);
	component_pool.pool_changed = true;

	char* component_pool_data = (char*)component_pool.component_pool_data;

	Component* new_component = new(component_pool_data + entity * sizeof(Component)) Component(std::forward<Args>(args)...);

	return *new_component;
}

template<typename Component>
inline bool EntityManager::HasComponent(Entity entity)
{
	assert(EntityExists(entity));

	ComponentPool& component_pool = GetComponentPool<Component>();

	return component_pool.m_component_pool_entities.find(entity) != component_pool.m_component_pool_entities.end();
}

template<typename Component>
inline Component& EntityManager::GetComponent(Entity entity)
{
	assert(EntityExists(entity));

	ComponentPool& component_pool = GetComponentPool<Component>();

	assert(HasComponent(entity, component_pool));

	char* component_pool_data = (char*)component_pool.component_pool_data;
	Component* component = (Component*)(component_pool_data + entity * sizeof(Component));
	
	return *component;
}

template<typename Component>
inline void EntityManager::RemoveComponent(Entity entity)
{
	assert(EntityExists(entity));

	ComponentPool& component_pool = GetComponentPool<Component>();

	assert(HasComponent(entity, component_pool));

	component_pool.m_component_pool_entities.erase(entity);
	component_pool.pool_changed = true;
}
