#pragma once
#include "Vendor/Include/Box2D/box2d.h"
#include "ECS/EntityDefinition.h"
#include "SceneSystem/SceneDefines.h"

class PhysicsContactListener : public b2ContactListener
{
private:
	struct CollisionData
	{
		Entity body_1_entity;
		SceneIndex body_1_scene_index;
		Entity body_2_entity;
		SceneIndex body_2_scene_index;
	};

private:
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

