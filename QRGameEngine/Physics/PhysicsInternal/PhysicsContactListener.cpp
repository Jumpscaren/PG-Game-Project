#include "pch.h"
#include "PhysicsContactListener.h"
#include "Physics/PhysicsCore.h"
#include "Event/EventCore.h"
#include "SceneSystem/SceneManager.h"
#include "ECS/EntityManager.h"
#include "Physics/PhysicsCore.h"

void PhysicsContactListener::BeginContact(b2Contact* contact)
{
	if (contact->GetFixtureA()->GetBody()->GetUserData().pointer && contact->GetFixtureB()->GetBody()->GetUserData().pointer)
	{
		auto body_1 = PhysicsCore::Get()->GetEntityAndSceneFromUserData((void*)contact->GetFixtureA()->GetBody()->GetUserData().pointer);
		auto body_2 = PhysicsCore::Get()->GetEntityAndSceneFromUserData((void*)contact->GetFixtureB()->GetBody()->GetUserData().pointer);

		//std::cout << "X = " << contact->GetManifold()->localNormal.x << ", Y = " << contact->GetManifold()->localNormal.y << "\n";

		if (!PhysicsCore::Get()->IsThreaded())
			EventCore::Get()->SendEvent("BeginCollision", body_1.first, body_1.second, body_2.first, body_2.second);
		else
			m_deferred_begin_collision_data.push_back(CollisionData(body_1.first, body_1.second, body_2.first, body_2.second));



		//EventCore::Get()->SendEvent("");
		//Send Event That a collision has happened, scripting system or something else could listen and catch these events
	}

	if (contact->GetFixtureA()->GetBody()->GetType() == b2_kinematicBody && contact->GetFixtureB()->GetBody()->GetType() == b2_dynamicBody)
	{
		std::cout << "Hi1\n";
	}
	if (contact->GetFixtureA()->GetBody()->GetType() == b2_dynamicBody && contact->GetFixtureB()->GetBody()->GetType() == b2_kinematicBody)
	{
		std::cout << "Hi2\n";
	}
}

void PhysicsContactListener::EndContact(b2Contact* contact)
{
	//Make thread safe

	if (contact->GetFixtureA()->GetBody()->GetUserData().pointer && contact->GetFixtureB()->GetBody()->GetUserData().pointer)
	{
		auto body_1 = PhysicsCore::Get()->GetEntityAndSceneFromUserData((void*)contact->GetFixtureA()->GetBody()->GetUserData().pointer);
		auto body_2 = PhysicsCore::Get()->GetEntityAndSceneFromUserData((void*)contact->GetFixtureB()->GetBody()->GetUserData().pointer);

		if (!PhysicsCore::Get()->IsThreaded())
			EventCore::Get()->SendEvent("EndCollision", body_1.first, body_1.second, body_2.first, body_2.second);
		else
			m_deferred_end_collision_data.push_back(CollisionData(body_1.first, body_1.second, body_2.first, body_2.second));

		//Send Event That a collision has exited, scripting system or something else could listen and catch these events
	}
}

void PhysicsContactListener::HandleDeferredCollisionData()
{
	for (uint64_t i = 0; i < m_deferred_begin_collision_data.size(); ++i)
	{
		const auto collision_data = m_deferred_begin_collision_data[i];

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
		const auto collision_data = m_deferred_end_collision_data[i];

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
