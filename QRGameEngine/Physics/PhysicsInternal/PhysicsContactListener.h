#pragma once
#include "Vendor/Include/Box2D/box2d.h"
#include "ECS/EntityDefinition.h"
#include "SceneSystem/SceneDefines.h"
#include "Common/EngineTypes.h"

class PhysicsContactListener : public b2ContactListener
{
private:
	struct CollisionData
	{
		Entity body_1_entity;
		SceneIndex body_1_scene_index;
		Entity body_2_entity;
		SceneIndex body_2_scene_index;

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

private:
	std::unordered_map<CollisionData, int, CollisionDataHasher> m_collisions_per_entity;

	std::vector<CollisionData> m_deferred_begin_collision_data;
	std::vector<CollisionData> m_deferred_end_collision_data;

private:
	/// Called when two fixtures begin to touch.
	void BeginContact(b2Contact* contact) override;

	/// Called when two fixtures cease to touch.
	void EndContact(b2Contact* contact) override;

public:
	void HandleDeferredCollisionData();
};

