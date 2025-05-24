#pragma once
#include "ECS/EntityDefinition.h"
#include "SceneDefines.h"

class EntityManager;

class SceneHierarchy
{
public:
	struct Child
	{
		Entity child;
		bool forced_position;
	};

private:
	struct Parent
	{
		Parent(Entity parent_entity) : parent(parent_entity), children({}), parented_children({}) {}
		Parent(Entity parent_entity, Entity child_entity) : parent(parent_entity), children({Child{.child = child_entity, .forced_position = false}}), parented_children({child_entity}) {}

		Entity parent;
		std::vector<Child> children;
		qr::unordered_set<Entity> parented_children;
	};

private:
	static SceneHierarchy* s_singleton;
	struct ParentSceneHierarchy
	{
		qr::unordered_set<Entity> root;
		qr::unordered_map<Entity, Parent> parent_connections;
	};
	qr::unordered_map<SceneIndex, ParentSceneHierarchy> m_parent_scene_hierarchies;

private:
	void RemoveParentChildRelationInternal(const Entity parent, const Entity child, ParentSceneHierarchy& parent_scene_hierarchy, EntityManager* entity_manager);

	void UpdateWorldTransforms(Entity child, ParentSceneHierarchy& parent_scene_hierarchy, EntityManager* entity_manager);

	ParentSceneHierarchy& GetParentSceneHierarchy(const SceneIndex scene_index);
	const ParentSceneHierarchy& GetParentSceneHierarchy(const SceneIndex scene_index) const;

public:
	SceneHierarchy();
	static SceneHierarchy* const Get();

	void AddSceneHierarchy(SceneIndex scene_index);
	void RemoveSceneHierarchy(SceneIndex scene_index);

	void AddParentChildRelation(Entity parent, Entity child);
	void AddParentChildRelation(SceneIndex scene_index, Entity parent, Entity child);
	void RemoveParentChildRelation(Entity child);
	void RemoveParentChildRelation(SceneIndex scene_index, Entity child);
	bool ParentHasChildren(SceneIndex scene_index, Entity parent) const;
	const qr::unordered_set<Entity>& GetChildren(SceneIndex scene_index, Entity parent) const;
	const std::vector<Child>& GetOrderedChildren(SceneIndex scene_index, Entity parent) const;
	qr::unordered_set<Entity> GetAllChildren(SceneIndex scene_index, Entity parent) const;
	qr::unordered_map<Entity, std::vector<Entity>> GetAllRelations(SceneIndex scene_index) const;

	void UpdateEntityTransforms(SceneIndex scene_index);

	void RemoveDeferredRelations(EntityManager* entity_manager);
};
