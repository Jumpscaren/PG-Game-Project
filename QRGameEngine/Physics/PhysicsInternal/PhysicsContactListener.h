#pragma once
#include "Vendor/Include/Box2D/id.h"
#include "ECS/EntityDefinition.h"
#include "SceneSystem/SceneDefines.h"
#include "Common/EngineTypes.h"

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
			res = res * 31 + std::hash<Entity>()(k.body_1_entity);
			res = res * 31 + std::hash<SceneIndex>()(k.body_1_scene_index);
			res = res * 31 + std::hash<Entity>()(k.body_2_entity);
			res = res * 31 + std::hash<SceneIndex>()(k.body_2_scene_index);
			return res;
		}
	};

	struct CollisionBodyData
	{
		std::pair<Entity, SceneIndex> body_1;
		std::pair<Entity, SceneIndex> body_2;
	};

private:
	qr::unordered_map<CollisionData, int, CollisionDataHasher> m_collisions_per_entity;

	std::vector<CollisionData> m_deferred_begin_collision_data;
	std::vector<CollisionData> m_deferred_end_collision_data;
	std::vector<CollisionData> m_deferred_hit_collision_data;

private:
	void HandleBeginCollision(const b2ShapeId shape_a, const b2ShapeId shape_b, const bool is_sensor_collision);
	void BeginContacts();

	void HandleEndCollision(const b2ShapeId shape_a, const b2ShapeId shape_b, const bool is_sensor_collision);
	void EndContacts();

	std::optional<CollisionBodyData> GetCollisionBodyData(const b2ShapeId shape_a, const b2ShapeId shape_b);

public:
	void HandleContacts();

	void HandleDeferredCollisionData();

	void DeletedEntity(const SceneIndex scene_index, const Entity entity);
};

