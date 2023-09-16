#pragma once
#include "ECS/EntityDefinition.h"
#include "PhysicDefines.h"
#include "Common/EngineTypes.h"

class b2World;
class b2Body;
class b2Shape;
class b2Fixture;
class PhysicsDebugDraw;
class PhysicsContactListener;
class PhysicsDestructionListener;
class EntityManager;

class PhysicsCore
{
public:
	enum PhysicObjectBodyType
	{
		StaticBody = 0,
		KinematicBody,
		DynamicBody,
		EmptyBody = 10
	};

private:
	struct PhysicObjectData {
		b2Body* object_body;
		PhysicObjectBodyType object_body_type;
		Entity object_entity = NULL_ENTITY;

		b2Fixture* object_box_fixture = nullptr;
		b2Fixture* object_circle_fixture = nullptr;
	};

private:
	b2World* m_world;
	PhysicsDebugDraw* m_debug_draw;
	PhysicsContactListener* m_contact_listener;
	PhysicsDestructionListener* m_destruction_listener;

	static PhysicsCore* s_physics_core;

	static constexpr uint64_t MAX_PHYSIC_OBJECTS = 1000;
	std::vector<PhysicObjectData> m_physic_object_data;
	std::stack<PhysicObjectHandle> m_free_physic_object_handles;

private:
	b2Fixture* AddFixtureToPhysicObject(PhysicObjectHandle physic_object_handle, b2Shape* physic_object_shape, const PhysicObjectBodyType& physic_object_body_type, bool trigger);

	PhysicObjectHandle GetPhysicObjectHandle(EntityManager* entity_manager, Entity entity);

public:
	PhysicsCore();

	static PhysicsCore* Get();

	void testg();

	void Update();

	void SetWorldPhysicObjectData(EntityManager* entity_manager);
	void GetWorldPhysicObjectData(EntityManager* entity_manager);

	void AddPhysicObject(EntityManager* entity_manager, Entity entity, const PhysicObjectBodyType& physic_object_body_type);
	void AddBoxCollider(EntityManager* entity_manager, Entity entity, const Vector2& half_box_size, bool trigger = false);
	void AddCircleCollider(EntityManager* entity_manager, Entity entity, float circle_radius, bool trigger = false);
	void AddBoxPhysicObject(EntityManager* entity_manager, Entity entity, const PhysicObjectBodyType& physic_object_body_type, const Vector2& half_box_size, bool trigger = false);
	void AddCirclePhysicObject(EntityManager* entity_manager, Entity entity, const PhysicObjectBodyType& physic_object_body_type, float circle_radius, bool trigger = false);

	void RemovePhysicObject(EntityManager* entity_manager, Entity entity);
	void RemoveBoxCollider(EntityManager* entity_manager, Entity entity);
	void RemoveCircleCollider(EntityManager* entity_manager, Entity entity);

	void RemoveDeferredPhysicObjects(EntityManager* entity_manager);
};

