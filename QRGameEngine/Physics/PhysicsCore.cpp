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

#include <thread>

PhysicsCore* PhysicsCore::s_physics_core;

bool PhysicsCore::IsSteppingWorld()
{
	if (!m_threaded_physics)
		return false;

	return m_update_physics;
}

PhysicsCore::PhysicsCore(bool threaded_physics) : m_threaded_physics(threaded_physics)
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

	if (m_threaded_physics)
		m_physic_update_thread = new std::thread(&PhysicsCore::ThreadUpdatePhysic, this);
}

PhysicsCore* PhysicsCore::Get()
{
	return s_physics_core;
}

void PhysicsCore::AddDeferredPhysicObjectCreation(SceneIndex scene_index, Entity entity, const PhysicObjectBodyType& physic_object_body_type)
{
	DeferredPhysicObjectCreationData physic_object_creation_data;
	physic_object_creation_data.entity = entity;
	physic_object_creation_data.scene_index = scene_index;
	physic_object_creation_data.physic_object_body_type = physic_object_body_type;
	physic_object_creation_data.has_collider_data = false;
	m_deferred_physic_object_creations.push_back(physic_object_creation_data);
}

void PhysicsCore::AddBoxColliderDeferredPhysicObjectCreation(SceneIndex scene_index, Entity entity, const Vector2& half_box_size, bool trigger)
{
	DeferredPhysicObjectCreationData physic_object_creation_data;
	physic_object_creation_data.entity = entity;
	physic_object_creation_data.scene_index = scene_index;
	physic_object_creation_data.has_collider_data = true;
	physic_object_creation_data.is_box_collider = true;
	physic_object_creation_data.trigger = trigger;
	physic_object_creation_data.half_box_size = half_box_size;
	m_deferred_physic_object_creations.push_back(physic_object_creation_data);
}

void PhysicsCore::AddCircleColliderDeferredPhysicObjectCreation(SceneIndex scene_index, Entity entity, float circle_radius, bool trigger)
{
	DeferredPhysicObjectCreationData physic_object_creation_data;
	physic_object_creation_data.entity = entity;
	physic_object_creation_data.scene_index = scene_index;
	physic_object_creation_data.has_collider_data = true;
	physic_object_creation_data.is_box_collider = false;
	physic_object_creation_data.trigger = trigger;
	physic_object_creation_data.circle_radius = circle_radius;
	m_deferred_physic_object_creations.push_back(physic_object_creation_data);
}

void PhysicsCore::AddDeferredPhysicObjectDestruction(SceneIndex scene_index, Entity entity, bool is_collider, bool is_box_collider)
{
	DeferredPhysicObjectDestructionData deferred_physic_object_destruction_data;
	deferred_physic_object_destruction_data.entity = entity;
	deferred_physic_object_destruction_data.scene_index = scene_index;
	deferred_physic_object_destruction_data.is_collider = is_collider;
	deferred_physic_object_destruction_data.is_box_collider = is_box_collider;

	EntityManager* entity_manager = SceneManager::GetSceneManager()->GetEntityManager(scene_index);
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
	deferred_physic_object_destruction_data.physic_object_handle = physic_object_handle;

	m_deferred_physic_object_destructions.push_back(deferred_physic_object_destruction_data);
}

void PhysicsCore::HandleDeferredPhysicObjectCreationData()
{
	for (auto& creation_data : m_deferred_physic_object_creations)
	{
		if (!creation_data.has_collider_data)
		{
			AddPhysicObject(creation_data.scene_index, creation_data.entity, creation_data.physic_object_body_type);
		}
		else
		{
			if (creation_data.is_box_collider)
			{
				AddBoxCollider(creation_data.scene_index, creation_data.entity, creation_data.half_box_size, creation_data.trigger);
			}
			else
			{
				AddCircleCollider(creation_data.scene_index, creation_data.entity, creation_data.circle_radius, creation_data.trigger);
			}
		}
	}

	m_deferred_physic_object_creations.clear();
}

void PhysicsCore::HandleDeferredPhysicObjectDestructionData()
{
	for (auto& destruction_data : m_deferred_physic_object_destructions)
	{
		EntityManager* entity_manager = SceneManager::GetSceneManager()->GetEntityManager(destruction_data.scene_index);
		bool entity_exists = entity_manager->EntityExists(destruction_data.entity);
		bool only_remove_internal_data = !entity_exists;
		if (entity_exists)
		{
			PhysicObjectHandle physic_object_handle = -1;
			if (entity_manager->HasComponent<DynamicBodyComponent>(destruction_data.entity))
			{
				physic_object_handle = entity_manager->GetComponent<DynamicBodyComponent>(destruction_data.entity).physic_object_handle;
			}
			else if (entity_manager->HasComponent<StaticBodyComponent>(destruction_data.entity))
			{
				physic_object_handle = entity_manager->GetComponent<StaticBodyComponent>(destruction_data.entity).physic_object_handle;
			}
			assert(physic_object_handle != -1);

			//Means that the entity has been reused and created another physic object and therefore we do not want to destroy the new entity data and physic object
			only_remove_internal_data = destruction_data.physic_object_handle != physic_object_handle;
		}

		if (!destruction_data.is_collider)
		{
			if (!only_remove_internal_data)
				RemovePhysicObject(destruction_data.scene_index, destruction_data.entity);
			else
				RemovePhysicObjectInternal(destruction_data.physic_object_handle);
		}
		else
		{
			if (destruction_data.is_box_collider)
			{
				if (!only_remove_internal_data)
					RemoveBoxCollider(destruction_data.scene_index, destruction_data.entity);
				else
					RemoveBoxColliderInternal(destruction_data.physic_object_handle);
			}
			else
			{
				if (!only_remove_internal_data)
					RemoveCircleCollider(destruction_data.scene_index, destruction_data.entity);
				else
					RemoveCircleColliderInternal(destruction_data.physic_object_handle);
			}
		}
	}

	m_deferred_physic_object_destructions.clear();
}

const bool& PhysicsCore::IsThreaded() const
{
	return m_threaded_physics;
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

	//for (int i = 0; i < 100; ++i)
	//{
	//	b2BodyDef body_def;
	//	b2Vec2 pos;
	//	pos.x = 0.5f;
	//	pos.y = (float)i + -0.5f;
	//	body_def.position = pos;
	//	body_def.type = b2_dynamicBody;
	//	//body_def.type = b2_staticBody;
	//	b2PolygonShape shape;
	//	shape.SetAsBox(0.5f, 0.5f);
	//	auto body = m_world->CreateBody(&body_def);
	//	body->CreateFixture(&shape, 0.01f);
	//	body->GetTransform();
	//}
}

void PhysicsCore::ThreadUpdatePhysic()
{
	while (true)
	{
		if (m_update_physics)
		{
			//std::cout << "Update: " << PhysicsCore::update << "\n";
			m_physic_update_thread_mutex.lock();
			Update();
			m_physic_update_thread_mutex.unlock();
			m_update_physics = false;
		}
	}
}

void PhysicsCore::Update()
{
	m_time_since_last_update += (float)Time::GetDeltaTime();
	while (m_time_since_last_update > TIME_STEP)
	{
		m_world->Step(TIME_STEP, VELOCITY_ITERATIONS, POSITION_ITERATIONS);
		m_time_since_last_update -= TIME_STEP;
	}
}

void PhysicsCore::UpdatePhysics()
{
	if (m_threaded_physics)
		m_update_physics = true;
	else
		Update();
}

void PhysicsCore::DrawColliders()
{
	uint32 flags = 0;
	flags += true * b2Draw::e_shapeBit;
	m_debug_draw->SetFlags(flags);
	m_world->DebugDraw();
}

void PhysicsCore::HandleDeferredPhysicData()
{
	m_contact_listener->HandleDeferredCollisionData();
	HandleDeferredPhysicObjectCreationData();
	HandleDeferredPhysicObjectDestructionData();
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
	if (m_threaded_physics)
		m_physic_update_thread_mutex.lock();

	entity_manager->System<DynamicBodyComponent, TransformComponent>([&](const DynamicBodyComponent& dynamic_body, TransformComponent& transform)
		{
			 const b2Transform& transform_physic_object = m_physic_object_data[dynamic_body.physic_object_handle].object_body->GetTransform();
			 transform.SetPosition(Vector2(transform_physic_object.p.x, transform_physic_object.p.y));
			 Vector3 rotation = transform.GetRotationEuler();
			 rotation.z = transform_physic_object.q.GetAngle();
			 transform.SetRotation(rotation);
		});

	if (m_threaded_physics)
		m_physic_update_thread_mutex.unlock();
}

void PhysicsCore::AddBoxPhysicObject(SceneIndex scene_index, Entity entity, const PhysicObjectBodyType& physic_object_body_type, const Vector2& half_box_size, bool trigger)
{
	AddPhysicObject(scene_index, entity, physic_object_body_type);
	AddBoxCollider(scene_index, entity, half_box_size, trigger);
}

void PhysicsCore::AddCirclePhysicObject(SceneIndex scene_index, Entity entity, const PhysicObjectBodyType& physic_object_body_type, float circle_radius, bool trigger)
{
	AddPhysicObject(scene_index, entity, physic_object_body_type);
	AddCircleCollider(scene_index, entity, circle_radius, trigger);
}

void PhysicsCore::RemovePhysicObject(SceneIndex scene_index, Entity entity)
{
	RemoveBoxCollider(scene_index, entity);
	RemoveCircleCollider(scene_index, entity);

	if (IsSteppingWorld())
	{
		AddDeferredPhysicObjectDestruction(scene_index, entity, false, false);
		return;
	}

	EntityManager* entity_manager = SceneManager::GetSceneManager()->GetEntityManager(scene_index);

	assert(entity_manager);

	PhysicObjectHandle physic_object_handle = -1;
	if (entity_manager->HasComponent<DynamicBodyComponent>(entity))
	{
		physic_object_handle = entity_manager->GetComponent<DynamicBodyComponent>(entity).physic_object_handle;
		entity_manager->RemoveComponent<DynamicBodyComponent>(entity);
	}
	else if (entity_manager->HasComponent<StaticBodyComponent>(entity))
	{
		physic_object_handle = entity_manager->GetComponent<StaticBodyComponent>(entity).physic_object_handle;
		entity_manager->RemoveComponent<StaticBodyComponent>(entity);
	}

	RemovePhysicObjectInternal(physic_object_handle);
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

void PhysicsCore::RemovePhysicObjectInternal(PhysicObjectHandle physic_object_handle)
{
	assert(physic_object_handle != -1);
	m_free_physic_object_handles.push(physic_object_handle);

	PhysicObjectData& physic_object_data = m_physic_object_data[physic_object_handle];
	m_world->DestroyBody(physic_object_data.object_body);

	physic_object_data.object_body = nullptr;
	physic_object_data.object_body_type = PhysicObjectBodyType::EmptyBody;
	physic_object_data.object_entity = NULL_ENTITY;
}

void PhysicsCore::RemoveBoxColliderInternal(PhysicObjectHandle physic_object_handle)
{
	PhysicObjectData& physic_object_data = m_physic_object_data[physic_object_handle];
	physic_object_data.object_body->DestroyFixture(physic_object_data.object_box_fixture);
	physic_object_data.object_box_fixture = nullptr;
}

void PhysicsCore::RemoveCircleColliderInternal(PhysicObjectHandle physic_object_handle)
{
	PhysicObjectData& physic_object_data = m_physic_object_data[physic_object_handle];
	physic_object_data.object_body->DestroyFixture(physic_object_data.object_circle_fixture);
	physic_object_data.object_circle_fixture = nullptr;
}

void PhysicsCore::AddPhysicObject(SceneIndex scene_index, Entity entity, const PhysicObjectBodyType& physic_object_body_type)
{
	if (IsSteppingWorld())
	{
		AddDeferredPhysicObjectCreation(scene_index, entity, physic_object_body_type);
		return;
	}

	EntityManager* entity_manager = SceneManager::GetSceneManager()->GetEntityManager(scene_index);

	assert(entity_manager);
	assert(entity_manager->HasComponent<TransformComponent>(entity));

	TransformComponent& transform = entity_manager->GetComponent<TransformComponent>(entity);
	Vector3 transform_position = transform.GetPosition();
	b2Vec2 physic_object_position = {};
	physic_object_position.x = transform_position.x;
	physic_object_position.y = transform_position.y;

	assert(m_free_physic_object_handles.size() != 0);
	PhysicObjectHandle new_physic_object_handle = m_free_physic_object_handles.top();
	m_free_physic_object_handles.pop();

	b2BodyDef physic_body_def = {};
	physic_body_def.position = physic_object_position;
	physic_body_def.type = (b2BodyType)physic_object_body_type;
	physic_body_def.userData.pointer = (uintptr_t)&m_physic_object_data[new_physic_object_handle];

	PhysicObjectData& physic_object_data = m_physic_object_data[new_physic_object_handle];
	physic_object_data.object_body = m_world->CreateBody(&physic_body_def);
	physic_object_data.object_body_type = physic_object_body_type;
	physic_object_data.object_entity = entity;
	physic_object_data.object_scene_index = scene_index;

	if (physic_object_body_type == PhysicObjectBodyType::DynamicBody)
	{
		entity_manager->AddComponent<DynamicBodyComponent>(entity).physic_object_handle = new_physic_object_handle;
	}
	else if (physic_object_body_type == PhysicObjectBodyType::StaticBody)
	{
		entity_manager->AddComponent<StaticBodyComponent>(entity).physic_object_handle = new_physic_object_handle;
	}
}

void PhysicsCore::AddBoxCollider(SceneIndex scene_index, Entity entity, const Vector2& half_box_size, bool trigger)
{
	if (IsSteppingWorld())
	{
		AddBoxColliderDeferredPhysicObjectCreation(scene_index, entity, half_box_size, trigger);
		return;
	}

	EntityManager* entity_manager = SceneManager::GetSceneManager()->GetEntityManager(scene_index);

	const TransformComponent& transform = entity_manager->GetComponent<TransformComponent>(entity);
	const Vector3& scale = transform.GetScale();

	b2PolygonShape shape;
	shape.SetAsBox(half_box_size.x * scale.x, half_box_size.y * scale.y);

	PhysicObjectHandle physic_object_handle = GetPhysicObjectHandle(entity_manager, entity);

	PhysicObjectData& physic_object_data = m_physic_object_data[physic_object_handle];
	physic_object_data.object_box_fixture = AddFixtureToPhysicObject(physic_object_handle, &shape, physic_object_data.object_body_type, trigger);

	entity_manager->AddComponent<BoxColliderComponent>(entity).physic_object_handle = physic_object_handle;
}

void PhysicsCore::AddCircleCollider(SceneIndex scene_index, Entity entity, float circle_radius, bool trigger)
{
	if (IsSteppingWorld())
	{
		AddCircleColliderDeferredPhysicObjectCreation(scene_index, entity, circle_radius, trigger);
		return;
	}

	EntityManager* entity_manager = SceneManager::GetSceneManager()->GetEntityManager(scene_index);

	const TransformComponent& transform = entity_manager->GetComponent<TransformComponent>(entity);
	const Vector3& scale = transform.GetScale();

	b2CircleShape shape;
	shape.m_radius = circle_radius * scale.x;

	PhysicObjectHandle physic_object_handle = GetPhysicObjectHandle(entity_manager, entity);

	PhysicObjectData& physic_object_data = m_physic_object_data[physic_object_handle];
	physic_object_data.object_circle_fixture = AddFixtureToPhysicObject(physic_object_handle, &shape, physic_object_data.object_body_type, trigger);

	entity_manager->AddComponent<CircleColliderComponent>(entity).physic_object_handle = physic_object_handle;
}

void PhysicsCore::RemoveBoxCollider(SceneIndex scene_index, Entity entity)
{
	EntityManager* entity_manager = SceneManager::GetSceneManager()->GetEntityManager(scene_index);

	if (entity_manager->HasComponent<BoxColliderComponent>(entity))
	{
		if (IsSteppingWorld())
		{
			AddDeferredPhysicObjectDestruction(scene_index, entity, true, true);
			return;
		}

		PhysicObjectHandle physic_object_handle = entity_manager->GetComponent<BoxColliderComponent>(entity).physic_object_handle;
		entity_manager->RemoveComponent<BoxColliderComponent>(entity);
		RemoveBoxColliderInternal(physic_object_handle);
	}
}

void PhysicsCore::RemoveCircleCollider(SceneIndex scene_index, Entity entity)
{
	EntityManager* entity_manager = SceneManager::GetSceneManager()->GetEntityManager(scene_index);

	if (entity_manager->HasComponent<CircleColliderComponent>(entity))
	{
		if (IsSteppingWorld())
		{
			AddDeferredPhysicObjectDestruction(scene_index, entity, true, false);
			return;
		}

		PhysicObjectHandle physic_object_handle = entity_manager->GetComponent<CircleColliderComponent>(entity).physic_object_handle;
		entity_manager->RemoveComponent<CircleColliderComponent>(entity);
		RemoveCircleColliderInternal(physic_object_handle);
	}
}

void PhysicsCore::RemoveDeferredPhysicObjects(EntityManager* entity_manager)
{
	entity_manager->System<DeferredEntityDeletion, DynamicBodyComponent>([&](Entity entity, DeferredEntityDeletion, DynamicBodyComponent)
		{
			RemovePhysicObject(entity_manager->GetSceneIndex(), entity);
		});

	entity_manager->System<DeferredEntityDeletion, StaticBodyComponent>([&](Entity entity, DeferredEntityDeletion, StaticBodyComponent)
		{
			RemovePhysicObject(entity_manager->GetSceneIndex(), entity);
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

std::pair<Entity, SceneIndex> PhysicsCore::GetEntityAndSceneFromUserData(void* user_data) const
{
	//if (!user_data)
	//	return std::pair<NULL_ENTITY, 0>;

	PhysicObjectData* physic_object_data = (PhysicObjectData*)(user_data);
	return std::pair<Entity, SceneIndex>(physic_object_data->object_entity, physic_object_data->object_scene_index);
}
