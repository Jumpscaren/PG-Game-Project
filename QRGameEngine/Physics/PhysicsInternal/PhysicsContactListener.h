#pragma once
#include "Vendor/Include/Box2D/id.h"
#include "ECS/EntityDefinition.h"
#include "SceneSystem/SceneDefines.h"
#include "Common/EngineTypes.h"
#include "Physics/PhysicDefines.h"

class PhysicsContactListener
{
private:
	struct CollisionData
	{
		Entity body_1_entity;
		SceneIndex body_1_scene_index;
		Entity body_2_entity;
		SceneIndex body_2_scene_index;

		bool is_sensor_collision;

		bool operator ==(const CollisionData& in_col) const
		{
			if (body_1_entity == in_col.body_1_entity && body_2_entity == in_col.body_2_entity && body_1_scene_index == in_col.body_1_scene_index && body_2_scene_index == in_col.body_2_scene_index)
			{
				return true;
			}
			else if (body_1_entity == in_col.body_2_entity && body_2_entity == in_col.body_1_entity && body_1_scene_index == in_col.body_2_scene_index && body_2_scene_index == in_col.body_1_scene_index)
			{
				return true;
			}

			return false;
		}
	};

	struct CollisionDataHasher
	{
		std::size_t operator()(const CollisionData& k) const
		{
			std::size_t res = 17;
			res = res * 31 + std::hash<Entity>()(k.body_1_entity + k.body_2_entity);
			res = res * 31 + std::hash<SceneIndex>()(k.body_1_scene_index + k.body_2_scene_index);
			return res;
		}
	};

	struct CollisionBodyData
	{
		EntityAndSceneData body_1;
		EntityAndSceneData body_2;
	};

	struct EntityAndSceneDataHasher
	{
		std::size_t operator()(const EntityAndSceneData& k) const
		{
			std::size_t res = 17;
			res = res * 31 + std::hash<Entity>()(k.entity);
			res = res * 31 + std::hash<SceneIndex>()(k.scene_index);
			return res;
		}
	};

	struct InternalShapeId
	{
		b2ShapeId shape_id;

		InternalShapeId(const b2ShapeId shape_id) : shape_id(shape_id) {}

		bool operator ==(const InternalShapeId& other) const
		{
			return shape_id.index1 == other.shape_id.index1 && shape_id.world0 == other.shape_id.world0 && shape_id.generation == other.shape_id.generation;
		}
	};

	struct InternalShapeIdDataHasher
	{
		std::size_t operator()(const InternalShapeId& k) const
		{
			std::size_t res = 17;
			res = res * 31 + std::hash<int32_t>()(k.shape_id.index1);
			res = res * 31 + std::hash<uint16_t>()(k.shape_id.world0);
			res = res * 31 + std::hash<uint16_t>()(k.shape_id.generation);
			return res;
		}
	};

	struct ShapeIdAndCollisionData
	{
		b2ShapeId other_shape_id;
		EntityAndSceneData other_body;
		CollisionData collision_data;

		bool operator ==(const ShapeIdAndCollisionData& other) const
		{
			return InternalShapeId(other_shape_id) == InternalShapeId(other.other_shape_id);
		}
	};

	struct ShapeIdAndCollisionDataHasher
	{
		std::size_t operator()(const ShapeIdAndCollisionData& k) const
		{
			static InternalShapeIdDataHasher internal_shape_id_data_hasher;
			static EntityAndSceneDataHasher entity_and_scene_data_hasher;
			static CollisionDataHasher collision_data_hasher;

			std::size_t res = 17;
			res = res * 31 + internal_shape_id_data_hasher(InternalShapeId{ k.other_shape_id });
			//res = res * 31 + entity_and_scene_data_hasher(k.other_body);
			//res = res * 31 + collision_data_hasher(k.collision_data);
			return res;
		}
	};

	using ShapeIdAndCollisionDataMap = qr::unordered_set<ShapeIdAndCollisionData, ShapeIdAndCollisionDataHasher>;
	using ShapeToCollision = qr::unordered_map<InternalShapeId, ShapeIdAndCollisionDataMap, InternalShapeIdDataHasher>;

private:
	qr::unordered_map<CollisionData, int, CollisionDataHasher> m_collisions_per_entity;
	qr::unordered_map<EntityAndSceneData, ShapeToCollision, EntityAndSceneDataHasher> m_entity_body_to_collisions;

	std::vector<CollisionData> m_deferred_begin_collision_data;
	std::vector<CollisionData> m_deferred_end_collision_data;
	std::vector<CollisionData> m_deferred_hit_collision_data;

private:
	void HandleBeginCollision(const b2ShapeId shape_a, const b2ShapeId shape_b, const bool is_sensor_collision);
	void BeginContacts();

	void HandleEndCollision(const b2ShapeId shape_a, const b2ShapeId shape_b, const bool is_sensor_collision);
	void EndContacts();

	std::optional<CollisionBodyData> GetCollisionBodyData(const b2ShapeId shape_a, const b2ShapeId shape_b);

	void AddShapeCollision(const EntityAndSceneData& body, const b2ShapeId& shape, const ShapeIdAndCollisionData& shape_id_and_collision_data);
	bool HasShapeCollision(const EntityAndSceneData& body, const b2ShapeId& shape, const b2ShapeId& other_shape);
	void RemoveShapeCollision(const EntityAndSceneData& body, const b2ShapeId& body_shape, const b2ShapeId& other_shape);

public:
	void HandleContacts();

	void HandleDeferredCollisionData();

	void DeletedEntity(const SceneIndex scene_index, const Entity entity);

	void RemovedShape(const Entity entity, const SceneIndex scene_index, const b2ShapeId shape_id);
};

