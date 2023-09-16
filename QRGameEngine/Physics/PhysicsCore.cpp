#include "pch.h"
#include "PhysicsCore.h"

#include "Vendor/Include/Box2D/box2d.h"
#include "Renderer/RenderCore.h"
#include "PhysicsInternal/PhysicsDebugDraw.h"
#include "PhysicsInternal/PhysicsContactListener.h"
#include "PhysicsInternal/PhysicsDestructionListener.h"

#include "Components/TransformComponent.h"
#include "Components/DynamicBodyComponent.h"
#include "Components/StaticBodyComponent.h"
#include "Components/BoxColliderComponent.h"
#include "Components/CircleColliderComponent.h"

#include "Time/Time.h"

PhysicsCore* PhysicsCore::s_physics_core;

PhysicsCore::PhysicsCore()
{
	s_physics_core = this;

	m_debug_draw = new PhysicsDebugDraw();
	m_contact_listener = new PhysicsContactListener();
	m_destruction_listener = new PhysicsDestructionListener();

	b2Vec2 gravity;
	gravity.Set(0.0f, -9.82f);
	m_world = new b2World(gravity);

	m_world->SetDestructionListener(m_destruction_listener);
	m_world->SetContactListener(m_contact_listener);
	m_world->SetDebugDraw(m_debug_draw);

	m_physic_object_data.resize(MAX_PHYSIC_OBJECTS);
	for (PhysicObjectHandle i = MAX_PHYSIC_OBJECTS - 1; i > 0; --i)
	{
		m_free_physic_object_handles.push(i);
	}
	m_free_physic_object_handles.push(0);
}

PhysicsCore* PhysicsCore::Get()
{
	return s_physics_core;
}

void PhysicsCore::testg()
{
	{
		b2BodyDef body_def;
		b2Vec2 pos;
		pos.x = 0.5f;
		pos.y = 0.5f;
		body_def.position = pos;
		body_def.type = b2_dynamicBody;
		//body_def.type = b2_staticBody;
		b2PolygonShape shape;
		shape.SetAsBox(0.5f, 0.5f);
		auto body = m_world->CreateBody(&body_def);
		body->CreateFixture(&shape, 0.01f);
		body->GetTransform();
	}

	{
		b2BodyDef body_def;
		b2Vec2 pos;
		pos.x = 0.5f;
		pos.y = -0.5f;
		body_def.position = pos;
		body_def.type = b2_dynamicBody;
		//body_def.type = b2_staticBody;
		b2PolygonShape shape;
		shape.SetAsBox(0.5f, 0.5f);
		auto body = m_world->CreateBody(&body_def);
		body->CreateFixture(&shape, 0.01f);
		body->GetTransform();
	}

	//change this to fix triggers?
	//m_world->SetContactFilter
}

float time_since_last_update = 0.0f;
void PhysicsCore::Update()
{
	//Add accumulator?
	static const float time_step = 1.0f / 120.0f;
	time_since_last_update += Time::GetDeltaTime();
	while (time_since_last_update > time_step)
	{
		m_world->Step(time_step, 8, 3);
		time_since_last_update -= time_step;
	}
	uint32 flags = 0;
	flags += true * b2Draw::e_shapeBit;
	m_debug_draw->SetFlags(flags);
	m_world->DebugDraw();
}

void PhysicsCore::SetWorldPhysicObjectData(EntityManager* entity_manager)
{
	entity_manager->System<DynamicBodyComponent, TransformComponent>([&](const DynamicBodyComponent& dynamic_body, const TransformComponent& transform)
		{
			m_physic_object_data[dynamic_body.physic_object_handle].object_body->SetTransform(b2Vec2(transform.GetPosition().x, transform.GetPosition().y), transform.GetRotationEuler().z);
		});
}

void PhysicsCore::GetWorldPhysicObjectData(EntityManager* entity_manager)
{
	entity_manager->System<DynamicBodyComponent, TransformComponent>([&](const DynamicBodyComponent& dynamic_body, TransformComponent& transform)
		{
			 const b2Transform& transform_physic_object = m_physic_object_data[dynamic_body.physic_object_handle].object_body->GetTransform();
			 transform.SetPosition(Vector2(transform_physic_object.p.x, transform_physic_object.p.y));
			 Vector3 rotation = transform.GetRotationEuler();
			 rotation.z = transform_physic_object.q.GetAngle();
			 transform.SetRotation(rotation);
		});
}

void PhysicsCore::AddBoxPhysicObject(EntityManager* entity_manager, Entity entity, const PhysicObjectBodyType& physic_object_body_type, const Vector2& half_box_size, bool trigger)
{
	AddPhysicObject(entity_manager, entity, physic_object_body_type);
	AddBoxCollider(entity_manager, entity, half_box_size, trigger);
}

void PhysicsCore::AddCirclePhysicObject(EntityManager* entity_manager, Entity entity, const PhysicObjectBodyType& physic_object_body_type, float circle_radius, bool trigger)
{
	AddPhysicObject(entity_manager, entity, physic_object_body_type);
	AddCircleCollider(entity_manager, entity, circle_radius, trigger);
}

void PhysicsCore::RemovePhysicObject(EntityManager* entity_manager, Entity entity)
{
	assert(entity_manager);

	RemoveBoxCollider(entity_manager, entity);
	RemoveCircleCollider(entity_manager, entity);

	PhysicObjectHandle physic_object_handle = -1;
	if (entity_manager->HasComponent<DynamicBodyComponent>(entity))
	{
		physic_object_handle = entity_manager->GetComponent<DynamicBodyComponent>(entity).physic_object_handle;
		m_free_physic_object_handles.push(physic_object_handle);
		entity_manager->RemoveComponent<DynamicBodyComponent>(entity);
	}
	else if (entity_manager->HasComponent<StaticBodyComponent>(entity))
	{
		physic_object_handle = entity_manager->GetComponent<StaticBodyComponent>(entity).physic_object_handle;
		m_free_physic_object_handles.push(physic_object_handle);
		entity_manager->RemoveComponent<StaticBodyComponent>(entity);
	}
	assert(physic_object_handle != -1);

	PhysicObjectData& physic_object_data = m_physic_object_data[physic_object_handle];
	m_world->DestroyBody(physic_object_data.object_body);
}

b2Fixture* PhysicsCore::AddFixtureToPhysicObject(PhysicObjectHandle physic_object_handle, b2Shape* physic_object_shape, const PhysicObjectBodyType& physic_object_body_type, bool trigger)
{
	const PhysicObjectData& physic_object_data = m_physic_object_data[physic_object_handle];

	float density = 1.0f;
	if (physic_object_body_type == PhysicObjectBodyType::StaticBody)
		density = 0.0f;

	b2Fixture* fixture = physic_object_data.object_body->CreateFixture(physic_object_shape, density);
	fixture->SetSensor(trigger);
	return fixture;
}

void PhysicsCore::AddPhysicObject(EntityManager* entity_manager, Entity entity, const PhysicObjectBodyType& physic_object_body_type)
{
	assert(entity_manager);
	assert(entity_manager->HasComponent<TransformComponent>(entity));

	TransformComponent& transform = entity_manager->GetComponent<TransformComponent>(entity);
	Vector3 transform_position = transform.GetPosition();
	b2Vec2 physic_object_position = {};
	physic_object_position.x = transform_position.x;
	physic_object_position.y = transform_position.y;

	b2BodyDef physic_body_def = {};
	physic_body_def.position = physic_object_position;
	physic_body_def.type = (b2BodyType)physic_object_body_type;

	PhysicObjectHandle new_physic_object_handle = m_free_physic_object_handles.top();
	m_free_physic_object_handles.pop();

	PhysicObjectData& physic_object_data = m_physic_object_data[new_physic_object_handle];
	physic_object_data.object_body = m_world->CreateBody(&physic_body_def);
	physic_object_data.object_body_type = physic_object_body_type;
	physic_object_data.object_entity = entity;

	if (physic_object_body_type == PhysicObjectBodyType::DynamicBody)
	{
		entity_manager->AddComponent<DynamicBodyComponent>(entity).physic_object_handle = new_physic_object_handle;
	}
	else if (physic_object_body_type == PhysicObjectBodyType::StaticBody)
	{
		entity_manager->AddComponent<StaticBodyComponent>(entity).physic_object_handle = new_physic_object_handle;
	}
}

void PhysicsCore::AddBoxCollider(EntityManager* entity_manager, Entity entity, const Vector2& half_box_size, bool trigger)
{
	const TransformComponent& transform = entity_manager->GetComponent<TransformComponent>(entity);
	const Vector3& scale = transform.GetScale();

	b2PolygonShape shape;
	shape.SetAsBox(half_box_size.x * scale.x, half_box_size.y * scale.y);

	PhysicObjectHandle physic_object_handle = GetPhysicObjectHandle(entity_manager, entity);

	PhysicObjectData& physic_object_data = m_physic_object_data[physic_object_handle];
	physic_object_data.object_box_fixture = AddFixtureToPhysicObject(physic_object_handle, &shape, physic_object_data.object_body_type, trigger);

	entity_manager->AddComponent<BoxColliderComponent>(entity).physic_object_handle = physic_object_handle;
}

void PhysicsCore::AddCircleCollider(EntityManager* entity_manager, Entity entity, float circle_radius, bool trigger)
{
	const TransformComponent& transform = entity_manager->GetComponent<TransformComponent>(entity);
	const Vector3& scale = transform.GetScale();

	b2CircleShape shape;
	shape.m_radius = circle_radius * scale.x;

	PhysicObjectHandle physic_object_handle = GetPhysicObjectHandle(entity_manager, entity);

	PhysicObjectData& physic_object_data = m_physic_object_data[physic_object_handle];
	physic_object_data.object_circle_fixture = AddFixtureToPhysicObject(physic_object_handle, &shape, physic_object_data.object_body_type, trigger);

	entity_manager->AddComponent<CircleColliderComponent>(entity).physic_object_handle = physic_object_handle;
}

void PhysicsCore::RemoveBoxCollider(EntityManager* entity_manager, Entity entity)
{
	if (entity_manager->HasComponent<BoxColliderComponent>(entity))
	{
		PhysicObjectHandle physic_object_handle = entity_manager->GetComponent<BoxColliderComponent>(entity).physic_object_handle;
		entity_manager->RemoveComponent<BoxColliderComponent>(entity);
		PhysicObjectData& physic_object_data = m_physic_object_data[physic_object_handle];
		physic_object_data.object_body->DestroyFixture(physic_object_data.object_box_fixture);
	}
}

void PhysicsCore::RemoveCircleCollider(EntityManager* entity_manager, Entity entity)
{
	if (entity_manager->HasComponent<CircleColliderComponent>(entity))
	{
		PhysicObjectHandle physic_object_handle = entity_manager->GetComponent<CircleColliderComponent>(entity).physic_object_handle;
		entity_manager->RemoveComponent<CircleColliderComponent>(entity);
		PhysicObjectData& physic_object_data = m_physic_object_data[physic_object_handle];
		physic_object_data.object_body->DestroyFixture(physic_object_data.object_circle_fixture);
	}
}

void PhysicsCore::RemoveDeferredPhysicObjects(EntityManager* entity_manager)
{
	entity_manager->System<DeferredEntityDeletion, DynamicBodyComponent>([&](Entity entity, DeferredEntityDeletion, DynamicBodyComponent)
		{
			RemovePhysicObject(entity_manager, entity);
		});

	entity_manager->System<DeferredEntityDeletion, StaticBodyComponent>([&](Entity entity, DeferredEntityDeletion, StaticBodyComponent)
		{
			RemovePhysicObject(entity_manager, entity);
		});
}

PhysicObjectHandle PhysicsCore::GetPhysicObjectHandle(EntityManager* entity_manager, Entity entity)
{
	PhysicObjectHandle physic_object_handle = -1;
	if (entity_manager->HasComponent<DynamicBodyComponent>(entity))
	{
		physic_object_handle = entity_manager->GetComponent<DynamicBodyComponent>(entity).physic_object_handle;
	}
	else if (entity_manager->HasComponent<StaticBodyComponent>(entity))
	{
		physic_object_handle = entity_manager->GetComponent<StaticBodyComponent>(entity).physic_object_handle;
	}

	assert(physic_object_handle != -1);

	return physic_object_handle;
}
