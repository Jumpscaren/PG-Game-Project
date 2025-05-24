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

void SceneHierarchy::AddSceneHierarchy(const SceneIndex scene_index)
{
	m_parent_scene_hierarchies.insert({ scene_index, ParentSceneHierarchy() });
}

void SceneHierarchy::RemoveSceneHierarchy(SceneIndex scene_index)
{
	m_parent_scene_hierarchies.erase(scene_index);
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

	ParentSceneHierarchy& parent_scene_hierarchy = GetParentSceneHierarchy(scene_index);

	if (parent_scene_hierarchy.root.contains(child))
	{
		parent_scene_hierarchy.root.erase(child);
	}

	auto it_parent = parent_scene_hierarchy.parent_connections.find(parent);
	if (it_parent == parent_scene_hierarchy.parent_connections.end())
	{
		parent_scene_hierarchy.parent_connections.insert({ parent, {parent, child} });
		if (!SceneManager::GetSceneManager()->GetEntityManager(scene_index)->HasComponent<ParentComponent>(parent))
		{
			parent_scene_hierarchy.root.insert(parent);
		}
		return;
	}

	it_parent->second.children.push_back(Child{ .child = child, .forced_position = false });
	it_parent->second.parented_children.insert(child);
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

	RemoveParentChildRelationInternal(parent, child, GetParentSceneHierarchy(scene_index), entity_manager);
}

bool SceneHierarchy::ParentHasChildren(const SceneIndex scene_index, const Entity parent) const
{
	const ParentSceneHierarchy& parent_scene_hierarchy = GetParentSceneHierarchy(scene_index);
	const auto it_parent = parent_scene_hierarchy.parent_connections.find(parent);
	if (it_parent == parent_scene_hierarchy.parent_connections.end())
	{
		return false;
	}
	return it_parent->second.children.size();
}

const qr::unordered_set<Entity>& SceneHierarchy::GetChildren(const SceneIndex scene_index, const Entity parent) const
{
	const auto it_parent = GetParentSceneHierarchy(scene_index).parent_connections.find(parent);
	assert(it_parent != GetParentSceneHierarchy(scene_index).parent_connections.end());
	return it_parent->second.parented_children;
}

const std::vector<SceneHierarchy::Child>& SceneHierarchy::GetOrderedChildren(const SceneIndex scene_index, const Entity parent) const
{
	const auto it_parent = GetParentSceneHierarchy(scene_index).parent_connections.find(parent);
	assert(it_parent != GetParentSceneHierarchy(scene_index).parent_connections.end());
	return it_parent->second.children;
}

qr::unordered_set<Entity> SceneHierarchy::GetAllChildren(const SceneIndex scene_index, const Entity parent) const
{
	qr::unordered_set<Entity> children;
	std::vector<Entity> children_to_process;
	children_to_process.push_back(parent);

	const ParentSceneHierarchy& parent_scene_hierarchy = GetParentSceneHierarchy(scene_index);
	while (!children_to_process.empty())
	{
		const auto it_parent = parent_scene_hierarchy.parent_connections.find(children_to_process.front());
		if (it_parent != parent_scene_hierarchy.parent_connections.end())
		{
			for (const auto& child : it_parent->second.children)
			{
				children_to_process.push_back(child.child);
				children.insert(child.child);
			}
		}
		children_to_process.erase(children_to_process.begin());
	}

	return children;
}

qr::unordered_map<Entity, std::vector<Entity>> SceneHierarchy::GetAllRelations(SceneIndex scene_index) const
{
	qr::unordered_map<Entity, std::vector<Entity>> relations;

	for (auto it_parent : GetParentSceneHierarchy(scene_index).parent_connections)
	{
		std::vector<Entity> children;
		for (const Child& child : it_parent.second.children)
		{
			children.push_back(child.child);
		}
		relations.insert({ it_parent.first, children });
	}

	return relations;
}

void SceneHierarchy::RemoveParentChildRelationInternal(const Entity parent, const Entity child, ParentSceneHierarchy& parent_scene_hierarchy, EntityManager* entity_manager)
{
	auto it_parent = parent_scene_hierarchy.parent_connections.find(parent);
	assert(it_parent != parent_scene_hierarchy.parent_connections.end());
	if (it_parent != parent_scene_hierarchy.parent_connections.end())
	{
		it_parent->second.parented_children.erase(child);
		const auto removed_child = std::ranges::remove_if(it_parent->second.children, [&](const Child& child_data) { return child_data.child == child; });
		it_parent->second.children.erase(removed_child.begin(), removed_child.end());
		entity_manager->RemoveComponent<ParentComponent>(child);
		if (it_parent->second.children.size() == 0)
		{
			parent_scene_hierarchy.parent_connections.erase(parent);
			parent_scene_hierarchy.root.erase(parent);
			if (parent_scene_hierarchy.parent_connections.contains(child))
			{
				parent_scene_hierarchy.root.insert(child);
			}
		}
	}
}

void SceneHierarchy::UpdateWorldTransforms(const Entity parent, ParentSceneHierarchy& parent_scene_hierarchy, EntityManager* entity_manager)
{
	auto it_parent = parent_scene_hierarchy.parent_connections.find(parent);
	if (it_parent == parent_scene_hierarchy.parent_connections.end())
	{
		return;
	}
	const auto& parent_world_matrix = entity_manager->GetComponent<TransformComponent>(parent).world_matrix;
	for (const auto& child : it_parent->second.children)
	{
		TransformComponent& child_transform = entity_manager->GetComponent<TransformComponent>(child.child);
		child_transform.world_matrix = entity_manager->GetComponent<ParentComponent>(child.child).world_matrix * parent_world_matrix;
		PositionScaleRotation matrix_data = TransformComponentInterface::GetDataFromWorldMatrix(child_transform);
		child_transform.m_rotation = matrix_data.rotation;
		child_transform.m_scale = matrix_data.scale;
		UpdateWorldTransforms(child.child, parent_scene_hierarchy, entity_manager);
	}
}

SceneHierarchy::ParentSceneHierarchy& SceneHierarchy::GetParentSceneHierarchy(const SceneIndex scene_index)
{
	return m_parent_scene_hierarchies.at(scene_index);
}

const SceneHierarchy::ParentSceneHierarchy& SceneHierarchy::GetParentSceneHierarchy(const SceneIndex scene_index) const
{
	return m_parent_scene_hierarchies.at(scene_index);
}

void SceneHierarchy::UpdateEntityTransforms(const SceneIndex scene_index)
{
	EntityManager* const entity_manager = SceneManager::GetSceneManager()->GetEntityManager(scene_index);
	ParentSceneHierarchy& parent_scene_hierarchy = GetParentSceneHierarchy(scene_index);
	for (const auto& root_parent : parent_scene_hierarchy.root)
	{
		UpdateWorldTransforms(root_parent, parent_scene_hierarchy, entity_manager);
	}
}

void SceneHierarchy::RemoveDeferredRelations(EntityManager* entity_manager)
{
	ParentSceneHierarchy& parent_scene_hierarchy = GetParentSceneHierarchy(entity_manager->GetSceneIndex());
	entity_manager->System<DeferredEntityDeletion, ParentComponent>([&](const Entity entity, const DeferredEntityDeletion&, const ParentComponent& parent_component)
		{
			RemoveParentChildRelationInternal(parent_component.parent, entity, parent_scene_hierarchy, entity_manager);
		});
}
