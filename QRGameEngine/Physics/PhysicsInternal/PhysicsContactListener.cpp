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

		return CollisionBodyData{.body_1 = body_1, .body_2 = body_2};
	}

	return std::nullopt;
}

void PhysicsContactListener::HandleBeginCollision(const b2ShapeId shape_a, const b2ShapeId shape_b, const bool is_sensor_collision)
{
	if (const std::optional<CollisionBodyData> collision_body_data = GetCollisionBodyData(shape_a, shape_b))
	{
		const std::pair<Entity, SceneIndex> body_1 = collision_body_data->body_1;
		const std::pair<Entity, SceneIndex> body_2 = collision_body_data->body_2;

		if (!PhysicsCore::Get()->IsThreaded())
			EventCore::Get()->SendEvent("BeginCollision", body_1.first, body_1.second, body_2.first, body_2.second);
		else
			m_deferred_begin_collision_data.push_back(CollisionData(body_1.first, body_1.second, body_2.first, body_2.second, is_sensor_collision));
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

	//const auto hit_events = b2World_GetHitEventThreshold(PhysicsCore::Get()->GetWorldId());
	//for (int i = 0; i < contact_events.hitCount; ++i)
	//{
	//	const b2ContactHitEvent& contact = contact_events.hitEvents[i];

	//	HandleBeginCollision(contact.shapeIdA, contact.shapeIdB, false);
	//}
}

void PhysicsContactListener::HandleEndCollision(const b2ShapeId shape_a, const b2ShapeId shape_b, const bool is_sensor_collision)
{
	if (const std::optional<CollisionBodyData> collision_body_data = GetCollisionBodyData(shape_a, shape_b))
	{
		const std::pair<Entity, SceneIndex> body_1 = collision_body_data->body_1;
		const std::pair<Entity, SceneIndex> body_2 = collision_body_data->body_2;

		if (!PhysicsCore::Get()->IsThreaded())
			EventCore::Get()->SendEvent("EndCollision", body_1.first, body_1.second, body_2.first, body_2.second);
		else
			m_deferred_end_collision_data.push_back(CollisionData(body_1.first, body_1.second, body_2.first, body_2.second, is_sensor_collision));
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
			assert(false);
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
}
