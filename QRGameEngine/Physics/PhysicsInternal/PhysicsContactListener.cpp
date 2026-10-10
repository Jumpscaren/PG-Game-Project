#include "pch.h"
#include "PhysicsContactListener.h"
#include "Physics/PhysicsCore.h"
#include "Event/EventCore.h"
#include "SceneSystem/SceneManager.h"
#include "ECS/EntityManager.h"
#include "Vendor/Include/Box2D/IncludeBox2D.h"

std::optional<PhysicsContactListener::CollisionBodyData> PhysicsContactListener::GetCollisionBodyData(const b2ShapeId shape_a, const b2ShapeId shape_b)
{
	if (!b2Shape_IsValid(shape_a) || !b2Shape_IsValid(shape_b))
	{
		return std::nullopt;
	}

	void* body_A_user_data = b2Body_GetUserData(b2Shape_GetBody(shape_a));
	void* body_B_user_data = b2Body_GetUserData(b2Shape_GetBody(shape_b));

	if (body_A_user_data && body_B_user_data)
	{
		const auto body_1 = PhysicsCore::Get()->GetEntityAndSceneFromUserData(body_A_user_data);
		const auto body_2 = PhysicsCore::Get()->GetEntityAndSceneFromUserData(body_B_user_data);

		return CollisionBodyData{ .body_1 = body_1, .body_2 = body_2 };
	}

	return std::nullopt;
}

void PhysicsContactListener::AddShapeCollision(const EntityAndSceneData& body, const b2ShapeId& shape, const ShapeIdAndCollisionData& other_shape_id_and_collision_data)
{
	auto it = m_entity_body_to_collisions.find(body);
	if (it == m_entity_body_to_collisions.end())
	{
		it = m_entity_body_to_collisions.emplace(body, ShapeToCollision{}).first;
	}

	if (auto shape_it = it->second.find(shape); shape_it != it->second.end())
	{
		shape_it->second.emplace(other_shape_id_and_collision_data);
	}
	else
	{
		it->second.emplace(shape, ShapeIdAndCollisionDataMap{}).first->second.emplace(other_shape_id_and_collision_data);
	}
}

bool PhysicsContactListener::HasShapeCollision(const EntityAndSceneData& body, const b2ShapeId& shape, const b2ShapeId& other_shape)
{
	if (auto it = m_entity_body_to_collisions.find(body); it != m_entity_body_to_collisions.end())
	{
		if (auto shape_it = it->second.find(shape); shape_it != it->second.end())
		{
			return shape_it->second.contains(ShapeIdAndCollisionData{ .other_shape_id = other_shape });
		}
	}

	return false;
}

void PhysicsContactListener::RemoveShapeCollision(const EntityAndSceneData& body, const b2ShapeId& body_shape, const b2ShapeId& other_shape)
{
	if (auto it = m_entity_body_to_collisions.find(body); it != m_entity_body_to_collisions.end())
	{
		if (auto shape_it = it->second.find(body_shape); shape_it != it->second.end())
		{
			const auto removed = shape_it->second.erase(ShapeIdAndCollisionData{ .other_shape_id = other_shape });
			assert(removed);
		}
		else
		{
			assert(false);
		}
	}
	else
	{
		//assert(false);
	}
}

void PhysicsContactListener::HandleBeginCollision(const b2ShapeId shape_a, const b2ShapeId shape_b, const bool is_sensor_collision)
{
	if (const std::optional<CollisionBodyData> collision_body_data = GetCollisionBodyData(shape_a, shape_b))
	{
		const EntityAndSceneData body_1 = collision_body_data->body_1;
		const EntityAndSceneData body_2 = collision_body_data->body_2;

		const CollisionData collision_data(body_1.entity, body_1.scene_index, body_2.entity, body_2.scene_index, is_sensor_collision);

		if (HasShapeCollision(body_1, shape_a, shape_b))
		{
			return;
		}

		m_deferred_begin_collision_data.push_back(collision_data);

		const ShapeIdAndCollisionData shape_and_collision_data_for_body_2{ .other_shape_id = shape_b, .other_body = body_2, .collision_data = collision_data };
		AddShapeCollision(body_1, shape_a, shape_and_collision_data_for_body_2);

		const ShapeIdAndCollisionData shape_and_collision_data_for_body_1{ .other_shape_id = shape_a, .other_body = body_1, .collision_data = collision_data };
		AddShapeCollision(body_2, shape_b, shape_and_collision_data_for_body_1);
	}
}

void PhysicsContactListener::BeginContacts()
{
	const b2SensorEvents sensor_events = b2World_GetSensorEvents(PhysicsCore::Get()->GetWorldId());
	for (int i = 0; i < sensor_events.beginCount; ++i)
	{
		const b2SensorBeginTouchEvent& contact = sensor_events.beginEvents[i];

		HandleBeginCollision(contact.sensorShapeId, contact.visitorShapeId, true);
	}

	const b2ContactEvents contact_events = b2World_GetContactEvents(PhysicsCore::Get()->GetWorldId());
	for (int i = 0; i < contact_events.beginCount; ++i)
	{
		const b2ContactBeginTouchEvent& contact = contact_events.beginEvents[i];

		HandleBeginCollision(contact.shapeIdA, contact.shapeIdB, false);
	}
}

void PhysicsContactListener::HandleEndCollision(const b2ShapeId shape_a, const b2ShapeId shape_b, const bool is_sensor_collision)
{
	if (const std::optional<CollisionBodyData> collision_body_data = GetCollisionBodyData(shape_a, shape_b))
	{
		const EntityAndSceneData body_1 = collision_body_data->body_1;
		const EntityAndSceneData body_2 = collision_body_data->body_2;

		if (!HasShapeCollision(body_1, shape_a, shape_b))
		{
			return;
		}

		m_deferred_end_collision_data.push_back(CollisionData(body_1.entity, body_1.scene_index, body_2.entity, body_2.scene_index, is_sensor_collision));

		RemoveShapeCollision(body_1, shape_a, shape_b);
		RemoveShapeCollision(body_2, shape_b, shape_a);
	}
}

void PhysicsContactListener::EndContacts()
{
	const b2SensorEvents sensor_events = b2World_GetSensorEvents(PhysicsCore::Get()->GetWorldId());

	for (int i = 0; i < sensor_events.endCount; ++i)
	{
		const b2SensorEndTouchEvent& contact = sensor_events.endEvents[i];

		HandleEndCollision(contact.sensorShapeId, contact.visitorShapeId, true);
	}

	const b2ContactEvents contact_events = b2World_GetContactEvents(PhysicsCore::Get()->GetWorldId());

	for (int i = 0; i < contact_events.endCount; ++i)
	{
		const b2ContactEndTouchEvent& contact = contact_events.endEvents[i];

		HandleEndCollision(contact.shapeIdA, contact.shapeIdB, false);
	}
}

void PhysicsContactListener::HandleContacts()
{
	BeginContacts();
	EndContacts();
}

void PhysicsContactListener::HandleDeferredCollisionData()
{
	for (uint64_t i = 0; i < m_deferred_begin_collision_data.size(); ++i)
	{
		const auto& collision_data = m_deferred_begin_collision_data[i];

		if (const auto collision_entity = m_collisions_per_entity.find(collision_data); collision_entity != m_collisions_per_entity.end())
		{
			++collision_entity->second;
			if (collision_entity->second != 1)
			{
				continue;
			}
		}
		else
		{
			m_collisions_per_entity.insert({ collision_data, 1 });
		}

		if (!SceneManager::GetSceneManager()->SceneExists(collision_data.body_1_scene_index) ||
			!SceneManager::GetSceneManager()->SceneExists(collision_data.body_2_scene_index))
			continue;

		EntityManager* entity_manager_1 = SceneManager::GetSceneManager()->GetEntityManager(collision_data.body_1_scene_index);
		EntityManager* entity_manager_2 = SceneManager::GetSceneManager()->GetEntityManager(collision_data.body_2_scene_index);
		if (!entity_manager_1->EntityExists(collision_data.body_1_entity) || !entity_manager_2->EntityExists(collision_data.body_2_entity))
			continue;

		EventCore::Get()->SendEvent("BeginCollision", collision_data.body_1_entity, collision_data.body_1_scene_index,
			collision_data.body_2_entity, collision_data.body_2_scene_index);
	}

	for (uint64_t i = 0; i < m_deferred_end_collision_data.size(); ++i)
	{
		const auto& collision_data = m_deferred_end_collision_data[i];

		if (const auto collision_entity = m_collisions_per_entity.find(collision_data); collision_entity != m_collisions_per_entity.end())
		{
			--collision_entity->second;
			if (collision_entity->second != 0)
			{
				continue;
			}
			m_collisions_per_entity.erase(collision_entity);
		}
		else
		{
			//assert(false);
			std::cout << "Collision data not found" << std::endl;
			continue;
		}

		if (!SceneManager::GetSceneManager()->SceneExists(collision_data.body_1_scene_index) ||
			!SceneManager::GetSceneManager()->SceneExists(collision_data.body_2_scene_index))
			continue;

		EntityManager* entity_manager_1 = SceneManager::GetSceneManager()->GetEntityManager(collision_data.body_1_scene_index);
		EntityManager* entity_manager_2 = SceneManager::GetSceneManager()->GetEntityManager(collision_data.body_2_scene_index);
		if (!entity_manager_1->EntityExists(collision_data.body_1_entity) || !entity_manager_2->EntityExists(collision_data.body_2_entity))
			continue;

		EventCore::Get()->SendEvent("EndCollision", collision_data.body_1_entity, collision_data.body_1_scene_index,
			collision_data.body_2_entity, collision_data.body_2_scene_index);
	}

	m_deferred_begin_collision_data.clear();
	m_deferred_end_collision_data.clear();

	for (const auto& [collision_data, count] : m_collisions_per_entity)
	{
		if (collision_data.body_1_entity == 517 && collision_data.body_1_scene_index == 2)
		{
			std::cout << "Body 1 Entity 517 in Scene 2 Collision count: " << count << std::endl;
		}
		if (collision_data.body_2_entity == 517 && collision_data.body_2_scene_index == 2)
		{
			std::cout << "Body 2 Entity 517 in Scene 2 Collision count: " << count << std::endl;
		}
	}
}

void PhysicsContactListener::DeletedEntity(const SceneIndex scene_index, const Entity entity)
{
	static std::vector<CollisionData> collisions_to_remove;
	collisions_to_remove.clear();

	for (const auto& it : m_collisions_per_entity)
	{
		const CollisionData& collision_data = it.first;

		const bool is_body_1_deleted = collision_data.body_1_scene_index == scene_index && collision_data.body_1_entity == entity;
		const bool is_body_2_deleted = collision_data.body_2_scene_index == scene_index && collision_data.body_2_entity == entity;
		if (is_body_1_deleted)
		{
			collisions_to_remove.push_back(CollisionData{ .body_1_entity = entity, .body_1_scene_index = scene_index, .body_2_entity = collision_data.body_2_entity, .body_2_scene_index = collision_data.body_2_scene_index });
		}
		else if (is_body_2_deleted)
		{
			collisions_to_remove.push_back(CollisionData{ .body_1_entity = collision_data.body_1_entity, .body_1_scene_index = collision_data.body_1_scene_index, .body_2_entity = entity, .body_2_scene_index = scene_index });
		}
	}

	for (const CollisionData& collision_data : collisions_to_remove)
	{
		m_collisions_per_entity.erase(collision_data);
	}

	m_entity_body_to_collisions.erase(EntityAndSceneData{ .entity = entity, .scene_index = scene_index });
}

void PhysicsContactListener::RemovedShape(const Entity entity, const SceneIndex scene_index, const b2ShapeId shape_id)
{
	if (auto it = m_entity_body_to_collisions.find(EntityAndSceneData{ .entity = entity, .scene_index = scene_index }); it != m_entity_body_to_collisions.end())
	{
		if (auto shape_it = it->second.find(shape_id); shape_it != it->second.end())
		{
			for (const ShapeIdAndCollisionData& other_shape_from_collision : shape_it->second)
			{
				m_deferred_end_collision_data.push_back(other_shape_from_collision.collision_data);

				RemoveShapeCollision(other_shape_from_collision.other_body, other_shape_from_collision.other_shape_id, shape_id);
			}
			it->second.erase(shape_it);
		}
	}
}
