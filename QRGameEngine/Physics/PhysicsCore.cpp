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

#include "Event/EventCore.h"

#include <thread>

PhysicsCore* PhysicsCore::s_physics_core;

bool PhysicsCore::IsDeferringPhysicCalls()
{
	if (!m_threaded_physics)
		return false;

	return m_defer_physic_calls;
}

PhysicsCore::PhysicsCore(bool threaded_physics) : m_threaded_physics(threaded_physics)
{
	s_physics_core = this;

	m_debug_draw = new PhysicsDebugDraw();
	m_contact_listener = new PhysicsContactListener();
	m_destruction_listener = new PhysicsDestructionListener();

	b2Vec2 gravity;
	//gravity.Set(0.0f, -9.82f);
	gravity.Set(0.0f, 0.0f);
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

	m_defer_physic_calls = false;

	EventCore::Get()->ListenToEvent<PhysicsCore::AwakePhysicObjectsFromLoadedScene>("SceneLoaded", 0, PhysicsCore::AwakePhysicObjectsFromLoadedScene);
}

PhysicsCore* PhysicsCore::Get()
{
	return s_physics_core;
}

void PhysicsCore::AddDeferredPhysicObjectHandle(const uint64_t handle, const bool is_physic_object_creation_data)
{
	DeferredPhysicObjectHandle deferred_physic_object_handle(is_physic_object_creation_data, handle);
	m_deferred_physic_object_handles.push_back(deferred_physic_object_handle);
}

void PhysicsCore::AddDeferredPhysicObjectCreation(const SceneIndex scene_index, const Entity entity, const PhysicObjectBodyType& physic_object_body_type)
{
	DeferredPhysicObjectCreationData physic_object_creation_data;
	physic_object_creation_data.entity = entity;
	physic_object_creation_data.scene_index = scene_index;
	physic_object_creation_data.physic_object_body_type = physic_object_body_type;
	physic_object_creation_data.has_collider_data = false;
	m_deferred_physic_object_creations.push_back(physic_object_creation_data);

	AddDeferredPhysicObjectHandle(m_deferred_physic_object_creations.size()-1, true);
}

void PhysicsCore::AddBoxColliderDeferredPhysicObjectCreation(const SceneIndex scene_index, const Entity entity)
{
	DeferredPhysicObjectCreationData physic_object_creation_data;
	physic_object_creation_data.entity = entity;
	physic_object_creation_data.scene_index = scene_index;
	physic_object_creation_data.has_collider_data = true;
	physic_object_creation_data.is_box_collider = true;
	m_deferred_physic_object_creations.push_back(physic_object_creation_data);

	AddDeferredPhysicObjectHandle(m_deferred_physic_object_creations.size() - 1, true);
}

void PhysicsCore::AddCircleColliderDeferredPhysicObjectCreation(const SceneIndex scene_index, const Entity entity)
{
	DeferredPhysicObjectCreationData physic_object_creation_data;
	physic_object_creation_data.entity = entity;
	physic_object_creation_data.scene_index = scene_index;
	physic_object_creation_data.has_collider_data = true;
	physic_object_creation_data.is_box_collider = false;
	m_deferred_physic_object_creations.push_back(physic_object_creation_data);

	AddDeferredPhysicObjectHandle(m_deferred_physic_object_creations.size() - 1, true);
}

void PhysicsCore::AddDeferredPhysicObjectDestruction(const SceneIndex scene_index, const Entity entity, const PhysicObjectHandle physic_object_handle, const bool is_collider, const bool is_box_collider)
{
	DeferredPhysicObjectDestructionData deferred_physic_object_destruction_data;
	deferred_physic_object_destruction_data.entity = entity;
	deferred_physic_object_destruction_data.scene_index = scene_index;
	deferred_physic_object_destruction_data.is_collider = is_collider;
	deferred_physic_object_destruction_data.is_box_collider = is_box_collider;

	EntityManager* entity_manager = SceneManager::GetSceneManager()->GetEntityManager(scene_index);

	deferred_physic_object_destruction_data.physic_object_handle = physic_object_handle;

	m_deferred_physic_object_destructions.push_back(deferred_physic_object_destruction_data);

	AddDeferredPhysicObjectHandle(m_deferred_physic_object_destructions.size() - 1, false);
}

void PhysicsCore::HandleDeferredPhysicObjectHandleData()
{
	for (const auto& handle : m_deferred_physic_object_handles)
	{
		if (handle.is_physic_object_creation_data)
			HandleDeferredPhysicObjectCreationData(m_deferred_physic_object_creations[handle.index_to_physic_object_data]);
		else
			HandleDeferredPhysicObjectDestructionData(m_deferred_physic_object_destructions[handle.index_to_physic_object_data]);
	}

	m_deferred_physic_object_handles.clear();
	m_deferred_physic_object_creations.clear();
	m_deferred_physic_object_destructions.clear();
}

void PhysicsCore::HandleDeferredPhysicObjectCreationData(const DeferredPhysicObjectCreationData& creation_data)
{
	if (!creation_data.has_collider_data)
	{
		AddPhysicObject(creation_data.scene_index, creation_data.entity, creation_data.physic_object_body_type);
	}
	else
	{
		EntityManager* entity_manager = SceneManager::GetSceneManager()->GetEntityManager(creation_data.scene_index);
		if (creation_data.is_box_collider)
		{
			const BoxColliderComponent& box_collider = entity_manager->GetComponent<BoxColliderComponent>(creation_data.entity);
			AddBoxCollider(creation_data.scene_index, creation_data.entity, box_collider.half_box_size, box_collider.trigger);
		}
		else
		{
			const CircleColliderComponent& circle_collider = entity_manager->GetComponent<CircleColliderComponent>(creation_data.entity);
			AddCircleCollider(creation_data.scene_index, creation_data.entity, circle_collider.circle_radius, circle_collider.trigger);
		}
	}
}

void PhysicsCore::HandleDeferredPhysicObjectDestructionData(const DeferredPhysicObjectDestructionData& destruction_data)
{
	if (!destruction_data.is_collider)
	{
		RemovePhysicObjectInternal(destruction_data.physic_object_handle);
	}
	else
	{
		if (destruction_data.is_box_collider)
		{
			RemoveBoxColliderInternal(destruction_data.physic_object_handle);
		}
		else
		{
			RemoveCircleColliderInternal(destruction_data.physic_object_handle);
		}
	}
}

void PhysicsCore::AddBoxFixture(const SceneIndex scene_index, const Entity entity, const Vector2& half_box_size, const bool trigger)
{
	EntityManager* entity_manager = SceneManager::GetSceneManager()->GetEntityManager(scene_index);

	const TransformComponent& transform = entity_manager->GetComponent<TransformComponent>(entity);
	const Vector3& scale = transform.GetScale();

	b2PolygonShape shape;
	shape.SetAsBox(half_box_size.x * scale.x, half_box_size.y * scale.y);

	PhysicObjectHandle physic_object_handle = GetPhysicObjectHandle(entity_manager, entity);

	PhysicObjectData& physic_object_data = m_physic_object_data[physic_object_handle];
	physic_object_data.object_box_fixture = AddFixtureToPhysicObject(physic_object_handle, &shape, physic_object_data.object_body_type, trigger);

	BoxColliderComponent& box_collider = entity_manager->GetComponent<BoxColliderComponent>(entity);
	box_collider.physic_object_handle = physic_object_handle;
	box_collider.half_box_size = half_box_size;
	box_collider.trigger = trigger;
}

void PhysicsCore::AddCircleFixture(const SceneIndex scene_index, const Entity entity, const float circle_radius, const bool trigger)
{
	EntityManager* entity_manager = SceneManager::GetSceneManager()->GetEntityManager(scene_index);

	const TransformComponent& transform = entity_manager->GetComponent<TransformComponent>(entity);
	const Vector3& scale = transform.GetScale();

	b2CircleShape shape;
	shape.m_radius = circle_radius * scale.x;

	PhysicObjectHandle physic_object_handle = GetPhysicObjectHandle(entity_manager, entity);

	PhysicObjectData& physic_object_data = m_physic_object_data[physic_object_handle];
	physic_object_data.object_circle_fixture = AddFixtureToPhysicObject(physic_object_handle, &shape, physic_object_data.object_body_type, trigger);

	CircleColliderComponent& circle_collider = entity_manager->GetComponent<CircleColliderComponent>(entity);
	circle_collider.physic_object_handle = physic_object_handle;
	circle_collider.circle_radius = circle_radius;
	circle_collider.trigger = trigger;
}

void PhysicsCore::AwakePhysicObjectsFromLoadedScene(const SceneIndex scene_index)
{
	EntityManager* const entity_manager = SceneManager::GetSceneManager()->GetEntityManager(scene_index);
	entity_manager->System<DynamicBodyComponent>([&](DynamicBodyComponent& dynamic_body)
		{
			dynamic_body.enabled = true;
			dynamic_body.awake = true;
		});
	entity_manager->System<StaticBodyComponent>([&](StaticBodyComponent& static_body)
		{
			static_body.enabled = true;
		});
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

void PhysicsCore::WaitForPhysics()
{
	if (m_threaded_physics)
	{
		m_physic_update_thread_mutex.lock();
		m_defer_physic_calls = false;
		m_physic_update_thread_mutex.unlock();
	}
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
	if (m_time_since_last_update > TIME_STEP * 10.0f)
		m_time_since_last_update = TIME_STEP * 10.0f;
	while (m_time_since_last_update > TIME_STEP)
	{
		//std::cout << "TIME: " << m_time_since_last_update << "\n";
		m_world->Step(TIME_STEP, VELOCITY_ITERATIONS, POSITION_ITERATIONS);
		m_time_since_last_update -= TIME_STEP;
	}
}

void PhysicsCore::UpdatePhysics()
{
	if (m_threaded_physics)
	{
		m_update_physics = true;
		m_defer_physic_calls = true;
	}
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
	HandleDeferredPhysicObjectHandleData();
}

void PhysicsCore::HandleDeferredCollisionData()
{
	m_contact_listener->HandleDeferredCollisionData();
}

void PhysicsCore::SetWorldPhysicObjectData(EntityManager* entity_manager)
{
	entity_manager->System<DynamicBodyComponent, TransformComponent>([&](const DynamicBodyComponent& dynamic_body, const TransformComponent& transform)
		{
			PhysicObjectData& physic_object = m_physic_object_data[dynamic_body.physic_object_handle];
			physic_object.object_body->SetTransform(b2Vec2(transform.GetPosition().x, transform.GetPosition().y), transform.GetRotationEuler().z);
			if (dynamic_body.awake != physic_object.object_body->IsAwake())
				physic_object.object_body->SetAwake(dynamic_body.awake);
			physic_object.object_body->SetLinearVelocity({ dynamic_body.velocity.x, dynamic_body.velocity.y });
			if (dynamic_body.fixed_rotation != physic_object.object_body->IsFixedRotation())
				physic_object.object_body->SetFixedRotation(dynamic_body.fixed_rotation);
			if (dynamic_body.enabled != physic_object.object_body->IsEnabled())
			{
				physic_object.object_body->SetEnabled(dynamic_body.enabled);
			}
		});

	//Timer timer;
	//timer.StartTimer();
	entity_manager->System<StaticBodyComponent, TransformComponent>([&](const StaticBodyComponent& static_body, const TransformComponent& transform)
		{
			PhysicObjectData& physic_object = m_physic_object_data[static_body.physic_object_handle];
			m_physic_object_data[static_body.physic_object_handle].object_body->SetTransform(b2Vec2(transform.GetPosition().x, transform.GetPosition().y), transform.GetRotationEuler().z);
			if (static_body.enabled != physic_object.object_body->IsEnabled())
			{
				physic_object.object_body->SetEnabled(static_body.enabled);
			}
		});
	//std::cout << "Time: " << timer.StopTimer() << "\n";

	entity_manager->System<BoxColliderComponent>([&](Entity entity, BoxColliderComponent& box_collider)
		{
			if (box_collider.update_box_collider) [[unlikely]]
			{
				RemoveBoxColliderInternal(box_collider.physic_object_handle);
				AddBoxFixture(entity_manager->GetSceneIndex(), entity, box_collider.half_box_size, box_collider.trigger);
				box_collider.update_box_collider = false;
			}
		});

	entity_manager->System<CircleColliderComponent>([&](Entity entity, CircleColliderComponent& circle_collider)
		{
			if (circle_collider.update_circle_collider) [[unlikely]]
			{
				RemoveCircleColliderInternal(circle_collider.physic_object_handle);
				AddCircleFixture(entity_manager->GetSceneIndex(), entity, circle_collider.circle_radius, circle_collider.trigger);
				circle_collider.update_circle_collider = false;
			}
		});
}

void PhysicsCore::GetWorldPhysicObjectData(EntityManager* entity_manager)
{
	entity_manager->System<DynamicBodyComponent, TransformComponent>([&](DynamicBodyComponent& dynamic_body, TransformComponent& transform)
		{
			 PhysicObjectData& physic_object = m_physic_object_data[dynamic_body.physic_object_handle];
			 const b2Transform& transform_physic_object = physic_object.object_body->GetTransform();
			 transform.SetPosition(Vector2(transform_physic_object.p.x, transform_physic_object.p.y));
			 Vector3 rotation = transform.GetRotationEuler();
			 rotation.z = transform_physic_object.q.GetAngle();
			 transform.SetRotation(rotation);

			 const b2Vec2& velocity = physic_object.object_body->GetLinearVelocity();
			 dynamic_body.velocity.x = velocity.x;
			 dynamic_body.velocity.y = velocity.y;
		});
}

void PhysicsCore::AddBoxPhysicObject(const SceneIndex scene_index, const Entity entity, const PhysicObjectBodyType& physic_object_body_type, const Vector2& half_box_size, const bool trigger)
{
	AddPhysicObject(scene_index, entity, physic_object_body_type);
	AddBoxCollider(scene_index, entity, half_box_size, trigger);
}

void PhysicsCore::AddCirclePhysicObject(const SceneIndex scene_index, const Entity entity, const PhysicObjectBodyType& physic_object_body_type, const float circle_radius, const bool trigger)
{
	AddPhysicObject(scene_index, entity, physic_object_body_type);
	AddCircleCollider(scene_index, entity, circle_radius, trigger);
}

void PhysicsCore::RemovePhysicObject(const SceneIndex scene_index, const Entity entity)
{
	EntityManager* entity_manager = SceneManager::GetSceneManager()->GetEntityManager(scene_index);

	if (entity_manager->HasComponent<BoxColliderComponent>(entity))
		entity_manager->RemoveComponent<BoxColliderComponent>(entity);
	if (entity_manager->HasComponent<CircleColliderComponent>(entity))
		entity_manager->RemoveComponent<CircleColliderComponent>(entity);

	assert(entity_manager);
	PhysicObjectHandle physic_object_handle = NULL_PHYSIC_OBJECT_HANDLE;
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

	if (IsDeferringPhysicCalls())
	{
		AddDeferredPhysicObjectDestruction(scene_index, entity, physic_object_handle, false, false);
		return;
	}

	RemovePhysicObjectInternal(physic_object_handle);
}

b2Fixture* PhysicsCore::AddFixtureToPhysicObject(const PhysicObjectHandle physic_object_handle, b2Shape* physic_object_shape, const PhysicObjectBodyType& physic_object_body_type, const bool trigger)
{
	const PhysicObjectData& physic_object_data = m_physic_object_data[physic_object_handle];

	float density = 1.0f;
	if (physic_object_body_type == PhysicObjectBodyType::StaticBody)
		density = 0.0f;

	b2Fixture* fixture = physic_object_data.object_body->CreateFixture(physic_object_shape, density);
	fixture->SetSensor(trigger);
	return fixture;
}

void PhysicsCore::RemovePhysicObjectInternal(const PhysicObjectHandle physic_object_handle)
{
	assert(physic_object_handle != NULL_PHYSIC_OBJECT_HANDLE);
	m_free_physic_object_handles.push(physic_object_handle);

	PhysicObjectData& physic_object_data = m_physic_object_data[physic_object_handle];
	m_world->DestroyBody(physic_object_data.object_body);

	physic_object_data.object_body = nullptr;
	physic_object_data.object_body_type = PhysicObjectBodyType::EmptyBody;
	physic_object_data.object_entity = NULL_ENTITY;
}

void PhysicsCore::RemoveBoxColliderInternal(const PhysicObjectHandle physic_object_handle)
{
	PhysicObjectData& physic_object_data = m_physic_object_data[physic_object_handle];
	physic_object_data.object_body->DestroyFixture(physic_object_data.object_box_fixture);
	physic_object_data.object_box_fixture = nullptr;
}

void PhysicsCore::RemoveCircleColliderInternal(const PhysicObjectHandle physic_object_handle)
{
	PhysicObjectData& physic_object_data = m_physic_object_data[physic_object_handle];
	physic_object_data.object_body->DestroyFixture(physic_object_data.object_circle_fixture);
	physic_object_data.object_circle_fixture = nullptr;
}

void PhysicsCore::AddPhysicObject(const SceneIndex scene_index, const Entity entity, const PhysicObjectBodyType& physic_object_body_type)
{
	EntityManager* entity_manager = SceneManager::GetSceneManager()->GetEntityManager(scene_index);

	assert(entity_manager);
	assert(entity_manager->HasComponent<TransformComponent>(entity));

	if (entity_manager->HasComponent<DynamicBodyComponent>(entity))
		assert(PhysicObjectBodyType::DynamicBody == physic_object_body_type);
	if (entity_manager->HasComponent<StaticBodyComponent>(entity))
		assert(PhysicObjectBodyType::StaticBody == physic_object_body_type);

	if (!entity_manager->HasComponent<DynamicBodyComponent>(entity) && physic_object_body_type == PhysicObjectBodyType::DynamicBody)
	{
		entity_manager->AddComponent<DynamicBodyComponent>(entity).physic_object_handle = NULL_PHYSIC_OBJECT_HANDLE;
	}
	else if (!entity_manager->HasComponent<StaticBodyComponent>(entity) && physic_object_body_type == PhysicObjectBodyType::StaticBody)
	{
		entity_manager->AddComponent<StaticBodyComponent>(entity).physic_object_handle = NULL_PHYSIC_OBJECT_HANDLE;
	}

	if (IsDeferringPhysicCalls())
	{
		AddDeferredPhysicObjectCreation(scene_index, entity, physic_object_body_type);
		return;
	}

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
		DynamicBodyComponent& dynamic_body = entity_manager->GetComponent<DynamicBodyComponent>(entity);
		dynamic_body.physic_object_handle = new_physic_object_handle;
#ifndef _EDITOR
		if (!SceneManager::GetSceneManager()->GetScene(scene_index)->IsSceneLoaded())
		{
			physic_object_data.object_body->SetEnabled(false);
			dynamic_body.enabled = physic_object_data.object_body->IsEnabled();
			physic_object_data.object_body->SetAwake(false);
			dynamic_body.awake = physic_object_data.object_body->IsAwake();
		}
#endif // !_EDITOR
#ifdef _EDITOR
		physic_object_data.object_body->SetEnabled(false);
		dynamic_body.enabled = physic_object_data.object_body->IsEnabled();
		physic_object_data.object_body->SetAwake(false);
		dynamic_body.awake = physic_object_data.object_body->IsAwake();
#endif // _EDITOR
	}
	else if (physic_object_body_type == PhysicObjectBodyType::StaticBody)
	{
		StaticBodyComponent& static_body = entity_manager->GetComponent<StaticBodyComponent>(entity);
		static_body.physic_object_handle = new_physic_object_handle;
#ifndef _EDITOR
		if (!SceneManager::GetSceneManager()->GetScene(scene_index)->IsSceneLoaded())
		{
			physic_object_data.object_body->SetEnabled(false);
			static_body.enabled = physic_object_data.object_body->IsEnabled();
		}
#endif // !_EDITOR
#ifdef _EDITOR
		physic_object_data.object_body->SetEnabled(false);
		static_body.enabled = physic_object_data.object_body->IsEnabled();
#endif // _EDITOR
	}
}

void PhysicsCore::AddBoxCollider(const SceneIndex scene_index, const Entity entity, const Vector2& half_box_size, const bool trigger)
{
	EntityManager* entity_manager = SceneManager::GetSceneManager()->GetEntityManager(scene_index);

	if (!entity_manager->HasComponent<BoxColliderComponent>(entity))
	{
		BoxColliderComponent& box_collider = entity_manager->AddComponent<BoxColliderComponent>(entity);
		box_collider.physic_object_handle = NULL_PHYSIC_OBJECT_HANDLE;
		box_collider.trigger = trigger;
		box_collider.half_box_size = half_box_size;
	}
	else
		assert(entity_manager->GetComponent<BoxColliderComponent>(entity).physic_object_handle == NULL_PHYSIC_OBJECT_HANDLE);

	if (IsDeferringPhysicCalls())
	{
		AddBoxColliderDeferredPhysicObjectCreation(scene_index, entity);
		return;
	}

	AddBoxFixture(scene_index, entity, half_box_size, trigger);
}

void PhysicsCore::AddCircleCollider(const SceneIndex scene_index, const Entity entity, const float circle_radius, const bool trigger)
{
	EntityManager* entity_manager = SceneManager::GetSceneManager()->GetEntityManager(scene_index);

	if (!entity_manager->HasComponent<CircleColliderComponent>(entity))
	{
		CircleColliderComponent& circle_collider = entity_manager->AddComponent<CircleColliderComponent>(entity);
		circle_collider.physic_object_handle = NULL_PHYSIC_OBJECT_HANDLE;
		circle_collider.trigger = trigger;
		circle_collider.circle_radius = circle_radius;
	}
	else
		assert(entity_manager->GetComponent<CircleColliderComponent>(entity).physic_object_handle == NULL_PHYSIC_OBJECT_HANDLE);

	if (IsDeferringPhysicCalls())
	{
		AddCircleColliderDeferredPhysicObjectCreation(scene_index, entity);
		return;
	}

	AddCircleFixture(scene_index, entity, circle_radius, trigger);
}

void PhysicsCore::RemoveBoxCollider(const SceneIndex scene_index, const Entity entity)
{
	EntityManager* entity_manager = SceneManager::GetSceneManager()->GetEntityManager(scene_index);

	if (entity_manager->HasComponent<BoxColliderComponent>(entity))
	{
		PhysicObjectHandle physic_object_handle = entity_manager->GetComponent<BoxColliderComponent>(entity).physic_object_handle;
		entity_manager->RemoveComponent<BoxColliderComponent>(entity);

		if (IsDeferringPhysicCalls())
		{
			AddDeferredPhysicObjectDestruction(scene_index, entity, physic_object_handle, true, true);
			return;
		}

		RemoveBoxColliderInternal(physic_object_handle);
	}
}

void PhysicsCore::RemoveCircleCollider(const SceneIndex scene_index, const Entity entity)
{
	EntityManager* entity_manager = SceneManager::GetSceneManager()->GetEntityManager(scene_index);

	if (entity_manager->HasComponent<CircleColliderComponent>(entity))
	{
		PhysicObjectHandle physic_object_handle = entity_manager->GetComponent<CircleColliderComponent>(entity).physic_object_handle;
		entity_manager->RemoveComponent<CircleColliderComponent>(entity);

		if (IsDeferringPhysicCalls())
		{
			AddDeferredPhysicObjectDestruction(scene_index, entity, physic_object_handle, true, false);
			return;
		}

		RemoveCircleColliderInternal(physic_object_handle);
	}
}

void PhysicsCore::RemoveDeferredPhysicObjects(EntityManager* entity_manager)
{
	entity_manager->System<DeferredEntityDeletion, DynamicBodyComponent>([&](const Entity entity, DeferredEntityDeletion, DynamicBodyComponent)
		{
			RemovePhysicObject(entity_manager->GetSceneIndex(), entity);
		});

	entity_manager->System<DeferredEntityDeletion, StaticBodyComponent>([&](const Entity entity, DeferredEntityDeletion, StaticBodyComponent)
		{
			RemovePhysicObject(entity_manager->GetSceneIndex(), entity);
		});
}

PhysicObjectHandle PhysicsCore::GetPhysicObjectHandle(EntityManager* entity_manager, const Entity entity)
{
	PhysicObjectHandle physic_object_handle = NULL_PHYSIC_OBJECT_HANDLE;
	if (entity_manager->HasComponent<DynamicBodyComponent>(entity))
	{
		physic_object_handle = entity_manager->GetComponent<DynamicBodyComponent>(entity).physic_object_handle;
	}
	else if (entity_manager->HasComponent<StaticBodyComponent>(entity))
	{
		physic_object_handle = entity_manager->GetComponent<StaticBodyComponent>(entity).physic_object_handle;
	}

	assert(physic_object_handle != NULL_PHYSIC_OBJECT_HANDLE);

	return physic_object_handle;
}

std::pair<Entity, SceneIndex> PhysicsCore::GetEntityAndSceneFromUserData(void* user_data) const
{
	PhysicObjectData* physic_object_data = (PhysicObjectData*)(user_data);
	return std::pair<Entity, SceneIndex>(physic_object_data->object_entity, physic_object_data->object_scene_index);
}
