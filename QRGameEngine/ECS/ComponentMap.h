#pragma once
#include "EntityManager.h"

class ComponentMap
{
public:
	struct ComponentMapData
	{
		void* component_map_method;
	};

private:
	static std::vector<ComponentMapData> s_component_maps;

private:
	template <typename Component>
	static void AddComponentToEntityManager(Entity entity, EntityManager* entity_manager)
	{
		entity_manager->AddComponent<Component>(entity);
		entity_manager->RemoveComponent<Component>(entity);
	}

	template <typename Component>
	static void AddComponent()
	{
		ComponentMapData component_map_data;
		component_map_data.component_map_method = (void*) (&ComponentMap::AddComponentToEntityManager<Component>);

		s_component_maps.push_back(component_map_data);
	}

public:
	static void AddComponentsToEntityManager(EntityManager* entity_manager);

	static void AddAllComponents();
};

