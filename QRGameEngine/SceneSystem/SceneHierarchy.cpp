#include "pch.h"
#include "SceneHierarchy.h"
#include "ECS/EntityManager.h"
#include "SceneManager.h"
#include "Components/ParentComponent.h"

SceneHierarchy* SceneHierarchy::s_singleton = nullptr;

SceneHierarchy::SceneHierarchy()
{
	s_singleton = this;
}

SceneHierarchy* const SceneHierarchy::Get()
{
	return s_singleton;
}

void SceneHierarchy::AddParentChildRelation(const Entity parent, const Entity child)
{
	const auto scene_index = SceneManager::GetSceneManager()->GetActiveSceneIndex();
	AddParentChildRelation(scene_index, parent, child);
}

void SceneHierarchy::AddParentChildRelation(const SceneIndex scene_index, const Entity parent, const Entity child)
{
	auto& parent_component = SceneManager::GetSceneManager()->GetEntityManager(scene_index)->AddComponent<ParentComponent>(child);
	parent_component.parent = parent;
	parent_component.world_matrix = SceneManager::GetSceneManager()->GetEntityManager(scene_index)->GetComponent<TransformComponent>(child).world_matrix;

	if (m_root.contains(child))
	{
		m_root.erase(child);
	}

	auto it_parent = m_parent_connections.find(parent);
	if (it_parent == m_parent_connections.end())
	{
		m_parent_connections.insert({ parent, {parent, child} });
		if (!SceneManager::GetSceneManager()->GetEntityManager(scene_index)->HasComponent<ParentComponent>(parent))
		{
			m_root.insert(parent);
		}
		return;
	}

	it_parent->second.children.insert(child);
}

void SceneHierarchy::RemoveParentChildRelation(const Entity child)
{
	const auto scene_index = SceneManager::GetSceneManager()->GetActiveSceneIndex();
	RemoveParentChildRelation(scene_index, child);
}

void SceneHierarchy::RemoveParentChildRelation(const SceneIndex scene_index, const Entity child)
{
	auto* const entity_manager = SceneManager::GetSceneManager()->GetEntityManager(scene_index);
	const auto parent = entity_manager->GetComponent<ParentComponent>(child).parent;

	RemoveParentChildRelationInternal(parent, child, entity_manager);
}

bool SceneHierarchy::ParentHasChildren(const SceneIndex scene_index, const Entity parent) const
{
	const auto it_parent = m_parent_connections.find(parent);
	if (it_parent == m_parent_connections.end())
	{
		return false;
	}
	return it_parent->second.children.size();
}

const std::set<Entity>& SceneHierarchy::GetChildren(const SceneIndex scene_index, const Entity parent) const
{
	const auto it_parent = m_parent_connections.find(parent);
	assert(it_parent != m_parent_connections.end());
	return it_parent->second.children;
}

std::set<Entity> SceneHierarchy::GetAllChildren(const SceneIndex scene_index, const Entity parent) const
{
	std::set<Entity> children;
	std::vector<Entity> children_to_process;
	children_to_process.push_back(parent);

	while (!children_to_process.empty())
	{
		const auto it_parent = m_parent_connections.find(children_to_process.front());
		if (it_parent != m_parent_connections.end())
		{
			for (const auto child : it_parent->second.children)
			{
				children_to_process.push_back(child);
				children.insert(child);
			}
		}
		children_to_process.erase(children_to_process.begin());
	}

	return children;
}

void SceneHierarchy::RemoveParentChildRelationInternal(const Entity parent, const Entity child, EntityManager* entity_manager)
{
	auto it_parent = m_parent_connections.find(parent);
	assert(it_parent != m_parent_connections.end());
	if (it_parent != m_parent_connections.end())
	{
		it_parent->second.children.erase(child);
		entity_manager->RemoveComponent<ParentComponent>(child);
		if (it_parent->second.children.size() == 0)
		{
			m_parent_connections.erase(parent);
			m_root.erase(parent);
			if (m_parent_connections.contains(child))
			{
				m_root.insert(child);
			}
		}
	}
}

void SceneHierarchy::UpdateWorldTransforms(const Entity parent, EntityManager* entity_manager)
{
	auto it_parent = m_parent_connections.find(parent);
	if (it_parent == m_parent_connections.end())
	{
		return;
	}
	const auto& parent_world_matrix = entity_manager->GetComponent<TransformComponent>(parent).world_matrix;
	for (const auto& child : it_parent->second.children)
	{
		entity_manager->GetComponent<TransformComponent>(child).world_matrix = entity_manager->GetComponent<ParentComponent>(child).world_matrix * parent_world_matrix;
		UpdateWorldTransforms(child, entity_manager);
	}
}

void SceneHierarchy::UpdateEntityTransforms(const SceneIndex scene_index)
{
	EntityManager* const entity_manager = SceneManager::GetSceneManager()->GetEntityManager(scene_index);
	for (const auto& root_parent : m_root)
	{
		UpdateWorldTransforms(root_parent, entity_manager);
	}
}

void SceneHierarchy::RemoveDeferredRelations(EntityManager* const entity_manager)
{
	entity_manager->System<DeferredEntityDeletion, ParentComponent>([&](const Entity entity, const DeferredEntityDeletion&, const ParentComponent& parent_component)
		{
			RemoveParentChildRelationInternal(parent_component.parent, entity, entity_manager);
		});
}
