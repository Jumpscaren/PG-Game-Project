#include "pch.h"
#include "ComponentMap.h"
#include "Components/AnimatableSpriteComponent.h"
#include "Components/BoxColliderComponent.h"
#include "Components/CameraComponent.h"
#include "Components/CircleColliderComponent.h"
#include "Components/DynamicBodyComponent.h"
#include "Components/EntityDataComponent.h"
#include "Components/ParentComponent.h"
#include "Components/PathFindingActorComponent.h"
#include "Components/PathFindingWorldComponent.h"
#include "Components/PureStaticBodyComponent.h"
#include "Components/ScriptComponent.h"
#include "Components/SpriteComponent.h"
#include "Components/StaticBodyComponent.h"
#include "Components/TransformComponent.h"
#include "Components/KinematicBodyComponent.h"

std::vector<ComponentMap::ComponentMapData> ComponentMap::s_component_maps;

void ComponentMap::AddComponentsToEntityManager(EntityManager* entity_manager)
{
	Entity entity = entity_manager->NewEntity();
	for (const auto& component_map : s_component_maps)
	{
		((void(*)(Entity, EntityManager*))(component_map.component_map_method))(entity, entity_manager);
	}
	entity_manager->RemoveEntity(entity);
}

void ComponentMap::AddAllComponents()
{
	AddComponent<AnimatableSpriteComponent>();
	AddComponent<BoxColliderComponent>();
	AddComponent<CameraComponent>();
	AddComponent<CircleColliderComponent>();
	AddComponent<DynamicBodyComponent>();
	AddComponent<EntityDataComponent>();
	AddComponent<ParentComponent>();
	AddComponent<PathFindingActorComponent>();
	AddComponent<PathFindingWorldComponent>();
	AddComponent<PureStaticBodyComponent>();
	AddComponent<ScriptComponent>();
	AddComponent<SpriteComponent>();
	AddComponent<StaticBodyComponent>();
	AddComponent<TransformComponent>();
	AddComponent<KinematicBodyComponent>();
}
