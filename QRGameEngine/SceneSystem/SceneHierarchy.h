#pragma once
#include "ECS/EntityDefinition.h"
#include "SceneDefines.h"

class EntityManager;

class SceneHierarchy
{
private:
	struct Parent
	{
		Parent(Entity parent_entity) : parent(parent_entity), children({}) {}
		Parent(Entity parent_entity, Entity child_entity) : parent(parent_entity), children({child_entity}) {}

		Entity parent;
		std::set<Entity> children;
	};

private:
	static SceneHierarchy* s_singleton;
	std::set<Entity> m_root;
	std::unordered_map<Entity, Parent> m_parent_connections;

private:
	void RemoveParentChildRelationInternal(const Entity parent, const Entity child, EntityManager* entity_manager);

	void UpdateWorldTransforms(Entity child, EntityManager* entity_manager);

public:
	SceneHierarchy();
	static SceneHierarchy* const Get();

	void AddParentChildRelation(Entity parent, Entity child);
	void AddParentChildRelation(SceneIndex scene_index, Entity parent, Entity child);
	void RemoveParentChildRelation(Entity child);
	void RemoveParentChildRelation(SceneIndex scene_index, Entity child);
	bool ParentHasChildren(SceneIndex scene_index, Entity parent) const;
	const std::set<Entity>& GetChildren(SceneIndex scene_index, Entity parent) const;
	std::set<Entity> GetAllChildren(SceneIndex scene_index, Entity parent) const;

	void UpdateEntityTransforms(SceneIndex scene_index);

	void RemoveDeferredRelations(EntityManager* const entity_manager);
};
