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

		if (!PhysicsCore::Get()->IsThreaded())
			EventCore::Get()->SendEvent("BeginCollision", body_1.first, body_1.second, body_2.first, body_2.second);
		else
			m_deferred_begin_collision_data.push_back(CollisionData(body_1.first, body_1.second, body_2.first, body_2.second));

		//EventCore::Get()->SendEvent("");
		//Send Event That a collision has happened, scripting system or something else could listen and catch these events
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
		if (!SceneManager::GetSceneManager()->SceneExists(m_deferred_begin_collision_data[i].body_1_scene_index) ||
			!SceneManager::GetSceneManager()->SceneExists(m_deferred_begin_collision_data[i].body_2_scene_index))
			continue;

		EntityManager* entity_manager_1 = SceneManager::GetSceneManager()->GetEntityManager(m_deferred_begin_collision_data[i].body_1_scene_index);
		EntityManager* entity_manager_2 = SceneManager::GetSceneManager()->GetEntityManager(m_deferred_begin_collision_data[i].body_2_scene_index);
		if (!entity_manager_1->EntityExists(m_deferred_begin_collision_data[i].body_1_entity) || !entity_manager_2->EntityExists(m_deferred_begin_collision_data[i].body_2_entity))
			continue;

		EventCore::Get()->SendEvent("BeginCollision", m_deferred_begin_collision_data[i].body_1_entity, m_deferred_begin_collision_data[i].body_1_scene_index, 
			m_deferred_begin_collision_data[i].body_2_entity, m_deferred_begin_collision_data[i].body_2_scene_index);
	}

	for (uint64_t i = 0; i < m_deferred_end_collision_data.size(); ++i)
	{
		if (!SceneManager::GetSceneManager()->SceneExists(m_deferred_end_collision_data[i].body_1_scene_index) ||
			!SceneManager::GetSceneManager()->SceneExists(m_deferred_end_collision_data[i].body_2_scene_index))
			continue;

		EntityManager* entity_manager_1 = SceneManager::GetSceneManager()->GetEntityManager(m_deferred_end_collision_data[i].body_1_scene_index);
		EntityManager* entity_manager_2 = SceneManager::GetSceneManager()->GetEntityManager(m_deferred_end_collision_data[i].body_2_scene_index);
		if (!entity_manager_1->EntityExists(m_deferred_end_collision_data[i].body_1_entity) || !entity_manager_2->EntityExists(m_deferred_end_collision_data[i].body_2_entity))
			continue;

		EventCore::Get()->SendEvent("EndCollision", m_deferred_end_collision_data[i].body_1_entity, m_deferred_end_collision_data[i].body_1_scene_index,
			m_deferred_end_collision_data[i].body_2_entity, m_deferred_end_collision_data[i].body_2_scene_index);
	}

	m_deferred_begin_collision_data.clear();
	m_deferred_end_collision_data.clear();
}
