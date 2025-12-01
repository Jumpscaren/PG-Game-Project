#include "pch.h"
#include "PhysicsCore.h"

#include "Vendor/Include/Box2D/box2d.h"
#include "Renderer/RenderCore.h"
#include "PhysicsInternal/PhysicsDebugDraw.h"
#include "PhysicsInternal/PhysicsContactFilter.h"
#include "PhysicsInternal/PhysicsContactListener.h"
#include "PhysicsInternal/PhysicsDestructionListener.h"

#include "Components/TransformComponent.h"
#include "Components/DynamicBodyComponent.h"
#include "Components/StaticBodyComponent.h"
#include "Components/PureStaticBodyComponent.h"
#include "Components/KinematicBodyComponent.h"
#include "Components/BoxColliderComponent.h"
#include "Components/CircleColliderComponent.h"
#include "Components/PolygonColliderComponent.h"

#include "Time/Time.h"

#include "Event/EventCore.h"

#include <thread>

PhysicsCore* PhysicsCore::s_physics_core;

namespace
{
	Vector2 GetPositionWithRotatedOffset(const Vector2& position, const Vector3& scale, const Vector2& offset, const float angle)
	{
		const Vector2 scaled_offset{ offset.x * scale.x, offset.y * scale.y };

		const float rotated_offset_x = scaled_offset.x * cosf(angle) - scaled_offset.y * sinf(angle);
		const float rotated_offset_y = scaled_offset.x * sinf(angle) + scaled_offset.y * cosf(angle);
		return Vector2(position.x + rotated_offset_x, position.y + rotated_offset_y);
	}

	Vector2 GetOffsetedPosition(EntityManager* entity_manager, const Entity entity, const TransformComponent& transform)
	{
		Vector2 offset;
		if (entity_manager->HasComponent<BoxColliderComponent>(entity))
		{
			offset = entity_manager->GetComponent<BoxColliderComponent>(entity).offset;
		}

		return GetPositionWithRotatedOffset(transform.GetPosition2D(), transform.GetScale(), offset, transform.GetRotationEuler().z);
	}
}

bool PhysicsCore::IsDeferringPhysicCalls()
{
	if (!m_threaded_physics)
		return false;

	return m_defer_physic_calls;
}

bool PhysicsCore::ShouldDeferPhysicCalls(const SceneIndex scene_index)
{
	return IsDeferringPhysicCalls() || 
		!m_handling_defered_physic_calls && m_threaded_physics && !SceneManager::GetSceneManager()->GetScene(scene_index)->IsSceneLoaded();
}

PhysicsCore::PhysicsCore(bool threaded_physics) : m_threaded_physics(threaded_physics)
{
	s_physics_core = this;

	m_debug_draw = new PhysicsDebugDraw();
	m_contact_filter = new PhysicsContactFilter();
	m_contact_listener = new PhysicsContactListener();
	m_destruction_listener = new PhysicsDestructionListener();

	b2WorldDeef

	b2Vec2 gravity;
	//gravity.Set(0.0f, -9.82f);
	gravity.Set(0.0f, 0.0f);
	m_world = new b2World(gravity);

	m_world->SetDebugDraw(m_debug_draw);
	//m_world->SetContactFilter(m_contact_filter);
	m_world->SetContactListener(m_contact_listener);
	m_world->SetDestructionListener(m_destruction_listener);

	m_physic_object_data.resize(MAX_PHYSIC_OBJECTS);
	for (PhysicObjectHandle i = MAX_PHYSIC_OBJECTS - 1; i > 0; --i)
	{
		m_free_physic_object_handles.push(i);
	}
	m_free_physic_object_handles.push(0);

	m_update_physics = PhysicThreadState::Wait;

	if (m_threaded_physics)
		m_physic_update_thread = new std::thread(&PhysicsCore::ThreadUpdatePhysic, this);

	m_defer_physic_calls = false;

	EventCore::Get()->ListenToEvent<PhysicsCore::AwakePhysicObjectsFromActivatedScene>("SceneActivated", 0, PhysicsCore::AwakePhysicObjectsFromActivatedScene);
}

PhysicsCore::~PhysicsCore()
{
	delete m_contact_listener;
	delete m_debug_draw;
	delete m_destruction_listener;
	delete m_world;
	if (m_threaded_physics)
	{
		m_update_physics = PhysicThreadState::Close;
		m_physic_update_thread->join();
		delete m_physic_update_thread;
	}
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
	physic_object_creation_data.collider_type = ColliderType::Box;
	m_deferred_physic_object_creations.push_back(physic_object_creation_data);

	AddDeferredPhysicObjectHandle(m_deferred_physic_object_creations.size() - 1, true);
}

void PhysicsCore::AddCircleColliderDeferredPhysicObjectCreation(const SceneIndex scene_index, const Entity entity)
{
	DeferredPhysicObjectCreationData physic_object_creation_data;
	physic_object_creation_data.entity = entity;
	physic_object_creation_data.scene_index = scene_index;
	physic_object_creation_data.has_collider_data = true;
	physic_object_creation_data.collider_type = ColliderType::Circle;
	m_deferred_physic_object_creations.push_back(physic_object_creation_data);

	AddDeferredPhysicObjectHandle(m_deferred_physic_object_creations.size() - 1, true);
}

void PhysicsCore::AddPolygonColliderDeferredPhysicObjectCreation(SceneIndex scene_index, Entity entity)
{
	DeferredPhysicObjectCreationData physic_object_creation_data;
	physic_object_creation_data.entity = entity;
	physic_object_creation_data.scene_index = scene_index;
	physic_object_creation_data.has_collider_data = true;
	physic_object_creation_data.collider_type = ColliderType::Polygon;
	m_deferred_physic_object_creations.push_back(physic_object_creation_data);

	AddDeferredPhysicObjectHandle(m_deferred_physic_object_creations.size() - 1, true);
}

void PhysicsCore::AddDeferredPhysicObjectDestruction(const SceneIndex scene_index, const Entity entity, const PhysicObjectHandle physic_object_handle, const bool is_collider, const ColliderType collider_type)
{
	DeferredPhysicObjectDestructionData deferred_physic_object_destruction_data;
	deferred_physic_object_destruction_data.entity = entity;
	deferred_physic_object_destruction_data.scene_index = scene_index;
	deferred_physic_object_destruction_data.is_collider = is_collider;
	deferred_physic_object_destruction_data.collider_type = collider_type;

	EntityManager* entity_manager = SceneManager::GetSceneManager()->GetEntityManager(scene_index);

	deferred_physic_object_destruction_data.physic_object_handle = physic_object_handle;

	m_deferred_physic_object_destructions.push_back(deferred_physic_object_destruction_data);

	AddDeferredPhysicObjectHandle(m_deferred_physic_object_destructions.size() - 1, false);
}

void PhysicsCore::HandleDeferredPhysicObjectHandleData()
{
	m_handling_defered_physic_calls = true;
	for (const auto& handle : m_deferred_physic_object_handles)
	{
		if (handle.is_physic_object_creation_data)
			HandleDeferredPhysicObjectCreationData(m_deferred_physic_object_creations[handle.index_to_physic_object_data]);
		else
			HandleDeferredPhysicObjectDestructionData(m_deferred_physic_object_destructions[handle.index_to_physic_object_data]);
	}
	m_handling_defered_physic_calls = false;

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

		if (creation_data.collider_type == ColliderType::Box)
		{
			const BoxColliderComponent& box_collider = entity_manager->GetComponent<BoxColliderComponent>(creation_data.entity);
			AddBoxCollider(creation_data.scene_index, creation_data.entity, box_collider.half_box_size, box_collider.trigger, box_collider.filter);
		}
		
		if (creation_data.collider_type == ColliderType::Circle)
		{
			const CircleColliderComponent& circle_collider = entity_manager->GetComponent<CircleColliderComponent>(creation_data.entity);
			AddCircleCollider(creation_data.scene_index, creation_data.entity, circle_collider.circle_radius, circle_collider.trigger, circle_collider.filter);
		}

		if (creation_data.collider_type == ColliderType::Polygon)
		{
			const PolygonColliderComponent& polygon_collider = entity_manager->GetComponent<PolygonColliderComponent>(creation_data.entity);
			AddPolygonCollider(creation_data.scene_index, creation_data.entity, polygon_collider.points, polygon_collider.loop, polygon_collider.solid, polygon_collider.trigger, polygon_collider.filter);
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
		switch (destruction_data.collider_type)
		{
		case ColliderType::Box:
			RemoveBoxColliderInternal(destruction_data.physic_object_handle);
			break;

		case ColliderType::Circle:
			RemoveCircleColliderInternal(destruction_data.physic_object_handle);
			break;

		case ColliderType::Polygon:
			RemovePolygonColliderInternal(destruction_data.physic_object_handle);
			break;
		}
	}
}

void PhysicsCore::AddBoxFixture(const SceneIndex scene_index, const Entity entity, const Vector2& half_box_size, const bool trigger, const ColliderFilter collider_filter)
{
	EntityManager* entity_manager = SceneManager::GetSceneManager()->GetEntityManager(scene_index);

	const TransformComponent& transform = entity_manager->GetComponent<TransformComponent>(entity);
	const Vector3 scale = transform.GetScale();

	b2PolygonShape shape;
	shape.SetAsBox(half_box_size.x * scale.x, half_box_size.y * scale.y);

	PhysicObjectHandle physic_object_handle = GetPhysicObjectHandle(entity_manager, entity);

	PhysicObjectData& physic_object_data = m_physic_object_data[physic_object_handle];
	physic_object_data.object_box_fixture = AddFixtureToPhysicObject(physic_object_handle, &shape, physic_object_data.object_body_type, trigger, collider_filter);

	BoxColliderComponent& box_collider = entity_manager->GetComponent<BoxColliderComponent>(entity);
	box_collider.physic_object_handle = physic_object_handle;
	box_collider.half_box_size = half_box_size;
	box_collider.trigger = trigger;
	box_collider.filter = collider_filter;
}

void PhysicsCore::AddCircleFixture(const SceneIndex scene_index, const Entity entity, const float circle_radius, const bool trigger, const ColliderFilter collider_filter)
{
	EntityManager* entity_manager = SceneManager::GetSceneManager()->GetEntityManager(scene_index);

	const TransformComponent& transform = entity_manager->GetComponent<TransformComponent>(entity);
	const Vector3 scale = transform.GetScale();

	b2CircleShape shape;
	shape.m_radius = circle_radius * scale.x;

	PhysicObjectHandle physic_object_handle = GetPhysicObjectHandle(entity_manager, entity);

	PhysicObjectData& physic_object_data = m_physic_object_data[physic_object_handle];
	physic_object_data.object_circle_fixture = AddFixtureToPhysicObject(physic_object_handle, &shape, physic_object_data.object_body_type, trigger, collider_filter);

	CircleColliderComponent& circle_collider = entity_manager->GetComponent<CircleColliderComponent>(entity);
	circle_collider.physic_object_handle = physic_object_handle;
	circle_collider.circle_radius = circle_radius;
	circle_collider.trigger = trigger;
	circle_collider.filter = collider_filter;
}

void PhysicsCore::AddPolygonFixture(SceneIndex scene_index, Entity entity, const std::vector<Vector2>& points, bool loop, bool solid, bool trigger, ColliderFilter collider_filter)
{
	EntityManager* entity_manager = SceneManager::GetSceneManager()->GetEntityManager(scene_index);

	const TransformComponent& transform = entity_manager->GetComponent<TransformComponent>(entity);
	const Vector3 scale = transform.GetScale();

	PhysicObjectHandle physic_object_handle = GetPhysicObjectHandle(entity_manager, entity);

	PhysicObjectData& physic_object_data = m_physic_object_data[physic_object_handle];

#ifdef _EDITOR
	std::vector<b2Vec2> b2_points;
	for (const auto& point : points)
	{
		b2_points.push_back(b2Vec2(point.x, point.y));
	}
	b2ChainShape shape;
	if (!loop)
	{
		shape.CreateChain(b2_points.data(), (int32)b2_points.size(), b2_points.front(), b2_points.back());
	}
	else
	{
		shape.CreateLoop(b2_points.data(), (int32)b2_points.size());
	}
	physic_object_data.object_polygon_fixtures.push_back(AddFixtureToPhysicObject(physic_object_handle, &shape, physic_object_data.object_body_type, trigger, collider_filter));
#else
	if (loop && solid)
	{
		const auto triangles = PolygonColliderComponentInterface::CreatePolygonTriangulation(entity, entity_manager);
		for (const auto& triangle : triangles)
		{
			std::vector<b2Vec2> b2_points;
			b2_points.push_back(b2Vec2(triangle.prev_point.x, triangle.prev_point.y));
			b2_points.push_back(b2Vec2(triangle.point.x, triangle.point.y));
			b2_points.push_back(b2Vec2(triangle.next_point.x, triangle.next_point.y));

			b2PolygonShape shape;
			shape.Set(b2_points.data(), (int32)b2_points.size());

			physic_object_data.object_polygon_fixtures.push_back(AddFixtureToPhysicObject(physic_object_handle, &shape, physic_object_data.object_body_type, trigger, collider_filter));
		}
	}
	else
	{
		std::vector<b2Vec2> b2_points;
		for (const auto& point : points)
		{
			b2_points.push_back(b2Vec2(point.x, point.y));
		}
		b2ChainShape shape;
		if (!loop)
		{
			shape.CreateChain(b2_points.data(), (int32)b2_points.size(), b2_points.front(), b2_points.back());
		}
		else
		{
			shape.CreateLoop(b2_points.data(), (int32)b2_points.size());
		}

		physic_object_data.object_polygon_fixtures.push_back(AddFixtureToPhysicObject(physic_object_handle, &shape, physic_object_data.object_body_type, trigger, collider_filter));
	}
#endif // _EDITOR

	PolygonColliderComponent& polygon_collider = entity_manager->GetComponent<PolygonColliderComponent>(entity);
	polygon_collider.physic_object_handle = physic_object_handle;
	polygon_collider.points = points;
	polygon_collider.trigger = trigger;
	polygon_collider.filter = collider_filter;
}

void PhysicsCore::AwakePhysicObjectsFromActivatedScene(const SceneIndex scene_index)
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
	entity_manager->System<PureStaticBodyComponent>([&](const Entity entity, PureStaticBodyComponent)
		{
			s_physics_core->AwakePureStaticBody(scene_index, entity);
		});
	entity_manager->System<KinematicBodyComponent>([&](KinematicBodyComponent& kinematic_body)
		{
			kinematic_body.enabled = true;
			kinematic_body.awake = true;
		});
}

void PhysicsCore::AwakePureStaticBody(SceneIndex scene_index, Entity entity)
{
	m_deferred_enable_pure_static_bodies.push_back(DeferredEnablePureStaticBody{.scene_index = scene_index, .entity = entity});
}

const bool& PhysicsCore::IsThreaded() const
{
	return m_threaded_physics;
}

void PhysicsCore::WaitForPhysics()
{
	if (m_threaded_physics)
	{
		m_physic_update_thread_mutex.lock();
		m_physic_update_thread_mutex.unlock();
		while (m_update_physics == PhysicThreadState::Update)
		{
		}
		m_defer_physic_calls = false;
	}

	EventCore::Get()->SendEvent("CurrentPhysicUpdateTicksCount", m_ticks_current_update);
}

void PhysicsCore::ThreadUpdatePhysic()
{
	while (true)
	{
		if (m_update_physics == PhysicThreadState::Update)
		{
			//std::cout << "Update: " << "\n";
			m_physic_update_thread_mutex.lock();
			Update();
			m_physic_update_thread_mutex.unlock();
			m_update_physics = PhysicThreadState::Wait;
		}

		if (m_update_physics == PhysicThreadState::Close)
		{
			return;
		}
	}
}

void PhysicsCore::Update()
{
	//To ensure that we don't end up in a endless death loop
	if (m_time_since_last_update > TIME_STEP * 10.0f)
		m_time_since_last_update = TIME_STEP * 10.0f;
	m_ticks_current_update = 0;
	while (m_time_since_last_update > TIME_STEP)
	{
		m_world->Step(TIME_STEP, VELOCITY_ITERATIONS, POSITION_ITERATIONS);
		m_time_since_last_update -= TIME_STEP;
		++m_ticks_current_update;
	}
}

void PhysicsCore::UpdatePhysics()
{
	m_time_since_last_update += (float)Time::GetDeltaTime();
	if (m_threaded_physics)
	{
		m_update_physics = PhysicThreadState::Update;
		m_defer_physic_calls = true;
	}
	else
	{
		Update();
	}
}

void PhysicsCore::DrawColliders(EntityManager* entity_manager)
{
	uint32 flags = 0;
	flags += true * b2Draw::e_shapeBit;
	m_debug_draw->SetFlags(flags);
	//m_world->DebugDraw();

	std::vector<b2Vec2> vertices;
	entity_manager->System<TransformComponent, BoxColliderComponent>([&](const Entity entity, const TransformComponent& transform, const BoxColliderComponent& box_collider)
		{
			if (!box_collider.debug_draw)
			{
				return;
			}

			const Vector2 position = transform.GetPosition2D() + box_collider.offset;

			const auto rotation_z = transform.GetRotationEuler().z;
			const Vector3& scale = transform.GetScale();

			const b2Vec2 top_left_corner = b2Vec2(-box_collider.half_box_size.x * scale.x, box_collider.half_box_size.y * scale.y);
			const b2Vec2 top_right_corner = b2Vec2(box_collider.half_box_size.x * scale.x, box_collider.half_box_size.y * scale.y);
			const b2Vec2 bottom_left_corner = b2Vec2(-box_collider.half_box_size.x * scale.x, -box_collider.half_box_size.y * scale.y);
			const b2Vec2 bottom_right_corner = b2Vec2(box_collider.half_box_size.x * scale.x, -box_collider.half_box_size.y * scale.y);

			const b2Vec2 rotated_top_left_corner = b2Vec2(top_left_corner.x * cosf(rotation_z) - top_left_corner.y * sinf(rotation_z), top_left_corner.x * sinf(rotation_z) + top_left_corner.y * cosf(rotation_z));
			const b2Vec2 rotated_top_right_corner = b2Vec2(top_right_corner.x * cosf(rotation_z) - top_right_corner.y * sinf(rotation_z), top_right_corner.x * sinf(rotation_z) + top_right_corner.y * cosf(rotation_z));
			const b2Vec2 rotated_bottom_left_corner = b2Vec2(bottom_left_corner.x * cosf(rotation_z) - bottom_left_corner.y * sinf(rotation_z), bottom_left_corner.x * sinf(rotation_z) + bottom_left_corner.y * cosf(rotation_z));
			const b2Vec2 rotated_bottom_right_corner = b2Vec2(bottom_right_corner.x * cosf(rotation_z) - bottom_right_corner.y * sinf(rotation_z), bottom_right_corner.x * sinf(rotation_z) + bottom_right_corner.y * cosf(rotation_z));

			const Vector2 offseted_position = GetOffsetedPosition(entity_manager, entity, transform);
			const b2Vec2 physic_position(offseted_position.x, offseted_position.y);

			vertices.push_back(physic_position + rotated_top_left_corner);
			vertices.push_back(physic_position + rotated_bottom_left_corner);
			vertices.push_back(physic_position + rotated_bottom_right_corner);
			vertices.push_back(physic_position + rotated_top_right_corner);

			m_debug_draw->DrawPolygon(vertices.data(), static_cast<int32>(vertices.size()), b2Color(1.0f, 0.0f, 0.0f));

			vertices.clear();
		});

	entity_manager->System<TransformComponent, CircleColliderComponent>([&](const TransformComponent& transform, const CircleColliderComponent& circle_collider)
		{
			if (!circle_collider.debug_draw)
			{
				return;
			}

			const Vector3 position = transform.GetPosition();

			const auto rotation_z = transform.GetRotationEuler().z;

			const b2Vec2 rotated_circle_pointer = b2Vec2(circle_collider.circle_radius * cosf(rotation_z), circle_collider.circle_radius * sinf(rotation_z));

			vertices.push_back(b2Vec2(position.x, position.y));
			vertices.push_back(b2Vec2(position.x, position.y) + rotated_circle_pointer);

			m_debug_draw->DrawPolygon(vertices.data(), static_cast<int32>(vertices.size()), b2Color(1.0f, 0.0f, 0.0f));
			m_debug_draw->DrawCircle(b2Vec2(position.x, position.y), circle_collider.circle_radius, b2Color(1.0f, 0.0f, 0.0f));

			vertices.clear();
		});

	entity_manager->System<TransformComponent, PolygonColliderComponent>([&](const TransformComponent& transform, const PolygonColliderComponent& polygon_collider)
		{
			if (!polygon_collider.debug_draw)
			{
				return;
			}

			const Vector3 position = transform.GetPosition();

			const auto rotation_z = transform.GetRotationEuler().z;

			const b2Vec2 position_2d(position.x, position.y);

			for (int i = 0; i < polygon_collider.points.size() - 1; ++i)
			{
				const Vector2 point_1 = polygon_collider.points[i];
				const Vector2 point_2 = polygon_collider.points[i + 1];

				m_debug_draw->DrawSegment(position_2d + b2Vec2(point_1.x, point_1.y), position_2d + b2Vec2(point_2.x, point_2.y), b2Color(1.0f, 0.0f, 0.0f));
			}

			if (polygon_collider.loop)
			{
				const Vector2 point_1 = polygon_collider.points.back();
				const Vector2 point_2 = polygon_collider.points.front();
				m_debug_draw->DrawSegment(position_2d + b2Vec2(point_1.x, point_1.y), position_2d + b2Vec2(point_2.x, point_2.y), b2Color(1.0f, 0.0f, 0.0f));
			}
		});
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
	const bool is_locked = m_world->IsLocked();

	entity_manager->System<DynamicBodyComponent, TransformComponent>([&](const DynamicBodyComponent& dynamic_body, const TransformComponent& transform)
		{
			const bool is_locked = m_world->IsLocked();
			const PhysicThreadState threat_run = m_update_physics;
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
	entity_manager->System<StaticBodyComponent, TransformComponent>([&](const Entity entity, const StaticBodyComponent& static_body, const TransformComponent& transform)
		{
			const bool is_locked = m_world->IsLocked();
			const PhysicThreadState threat_run = m_update_physics;
			PhysicObjectData& physic_object = m_physic_object_data[static_body.physic_object_handle];

			const float rotation_z = transform.GetRotationEuler().z;

			const Vector2 position = GetOffsetedPosition(entity_manager, entity, transform);

			physic_object.object_body->SetTransform(b2Vec2(position.x, position.y), rotation_z);
			if (static_body.enabled != physic_object.object_body->IsEnabled())
			{
				physic_object.object_body->SetEnabled(static_body.enabled);
			}
		});
	//std::cout << "Time: " << timer.StopTimer() << "\n";

	for (const auto& pure_static_body : m_deferred_enable_pure_static_bodies)
	{
		EntityManager* ent_man = SceneManager::GetSceneManager()->GetEntityManager(pure_static_body.scene_index);
		if (ent_man->EntityExists(pure_static_body.entity) && ent_man->HasComponent<PureStaticBodyComponent>(pure_static_body.entity))
		{
			const auto transform = ent_man->GetComponent<TransformComponent>(pure_static_body.entity);
			const auto physic_object_handle = ent_man->GetComponent<PureStaticBodyComponent>(pure_static_body.entity).physic_object_handle;
			PhysicObjectData& physic_object = m_physic_object_data[physic_object_handle];

			Vector2 offset;
			if (ent_man->HasComponent<BoxColliderComponent>(pure_static_body.entity))
			{
				offset = ent_man->GetComponent<BoxColliderComponent>(pure_static_body.entity).offset;
			}

			const Vector2 position = transform.GetPosition2D() + offset;
			physic_object.object_body->SetTransform(b2Vec2(position.x, position.y), transform.GetRotationEuler().z);
			physic_object.object_body->SetEnabled(true);
			physic_object.object_body->SetAwake(true);
		}
	}
	m_deferred_enable_pure_static_bodies.clear();

	entity_manager->System<KinematicBodyComponent, TransformComponent>([&](const KinematicBodyComponent& kinematic_body, const TransformComponent& transform)
		{
			PhysicObjectData& physic_object = m_physic_object_data[kinematic_body.physic_object_handle];
			physic_object.object_body->SetTransform(b2Vec2(transform.GetPosition().x, transform.GetPosition().y), transform.GetRotationEuler().z);
			if (kinematic_body.awake != physic_object.object_body->IsAwake())
				physic_object.object_body->SetAwake(kinematic_body.awake);
			physic_object.object_body->SetLinearVelocity({ kinematic_body.velocity.x, kinematic_body.velocity.y });
			if (kinematic_body.fixed_rotation != physic_object.object_body->IsFixedRotation())
				physic_object.object_body->SetFixedRotation(kinematic_body.fixed_rotation);
			if (kinematic_body.enabled != physic_object.object_body->IsEnabled())
			{
				physic_object.object_body->SetEnabled(kinematic_body.enabled);
			}
		});

	entity_manager->System<BoxColliderComponent>([&](Entity entity, BoxColliderComponent& box_collider)
		{
			if (box_collider.update_box_collider) [[unlikely]]
			{
				RemoveBoxColliderInternal(box_collider.physic_object_handle);
				AddBoxFixture(entity_manager->GetSceneIndex(), entity, box_collider.half_box_size, box_collider.trigger, box_collider.filter);
				box_collider.update_box_collider = false;
			}
		});

	entity_manager->System<CircleColliderComponent>([&](Entity entity, CircleColliderComponent& circle_collider)
		{
			if (circle_collider.update_circle_collider) [[unlikely]]
			{
				RemoveCircleColliderInternal(circle_collider.physic_object_handle);
				AddCircleFixture(entity_manager->GetSceneIndex(), entity, circle_collider.circle_radius, circle_collider.trigger, circle_collider.filter);
				circle_collider.update_circle_collider = false;
			}
		});

	entity_manager->System<PolygonColliderComponent>([&](Entity entity, PolygonColliderComponent& polygon_collider)
		{
			if (polygon_collider.update_polygon_collider) [[unlikely]]
				{
					RemovePolygonColliderInternal(polygon_collider.physic_object_handle);
					AddPolygonFixture(entity_manager->GetSceneIndex(), entity, polygon_collider.points, polygon_collider.loop, polygon_collider.solid, polygon_collider.trigger, polygon_collider.filter);
					polygon_collider.update_polygon_collider = false;
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

	entity_manager->System<KinematicBodyComponent, TransformComponent>([&](KinematicBodyComponent& kinematic_body, TransformComponent& transform)
		{
			PhysicObjectData& physic_object = m_physic_object_data[kinematic_body.physic_object_handle];
			const b2Transform& transform_physic_object = physic_object.object_body->GetTransform();
			transform.SetPosition(Vector2(transform_physic_object.p.x, transform_physic_object.p.y));
			Vector3 rotation = transform.GetRotationEuler();
			rotation.z = transform_physic_object.q.GetAngle();
			transform.SetRotation(rotation);

			const b2Vec2& velocity = physic_object.object_body->GetLinearVelocity();
			kinematic_body.velocity.x = velocity.x;
			kinematic_body.velocity.y = velocity.y;
		});
}

void PhysicsCore::AddBoxPhysicObject(const SceneIndex scene_index, const Entity entity, const PhysicObjectBodyType& physic_object_body_type, const Vector2& half_box_size, const bool trigger, const ColliderFilter collider_filter)
{
	AddPhysicObject(scene_index, entity, physic_object_body_type);
	AddBoxCollider(scene_index, entity, half_box_size, trigger, collider_filter);
}

void PhysicsCore::AddCirclePhysicObject(const SceneIndex scene_index, const Entity entity, const PhysicObjectBodyType& physic_object_body_type, const float circle_radius, const bool trigger, const ColliderFilter collider_filter)
{
	AddPhysicObject(scene_index, entity, physic_object_body_type);
	AddCircleCollider(scene_index, entity, circle_radius, trigger, collider_filter);
}

void PhysicsCore::RemovePhysicObject(const SceneIndex scene_index, const Entity entity)
{
	EntityManager* entity_manager = SceneManager::GetSceneManager()->GetEntityManager(scene_index);

	if (entity_manager->HasComponent<BoxColliderComponent>(entity))
		entity_manager->RemoveComponent<BoxColliderComponent>(entity);
	if (entity_manager->HasComponent<CircleColliderComponent>(entity))
		entity_manager->RemoveComponent<CircleColliderComponent>(entity);
	if (entity_manager->HasComponent<PolygonColliderComponent>(entity))
		entity_manager->RemoveComponent<PolygonColliderComponent>(entity);

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
	else if (entity_manager->HasComponent<PureStaticBodyComponent>(entity))
	{
		physic_object_handle = entity_manager->GetComponent<PureStaticBodyComponent>(entity).physic_object_handle;
		entity_manager->RemoveComponent<PureStaticBodyComponent>(entity);
	}
	else if (entity_manager->HasComponent<KinematicBodyComponent>(entity))
	{
		physic_object_handle = entity_manager->GetComponent<KinematicBodyComponent>(entity).physic_object_handle;
		entity_manager->RemoveComponent<KinematicBodyComponent>(entity);
	}

	if (ShouldDeferPhysicCalls(scene_index))
	{
		AddDeferredPhysicObjectDestruction(scene_index, entity, physic_object_handle, false, ColliderType::None);
		return;
	}

	RemovePhysicObjectInternal(physic_object_handle);
}

b2Fixture* PhysicsCore::AddFixtureToPhysicObject(const PhysicObjectHandle physic_object_handle, b2Shape* physic_object_shape, const PhysicObjectBodyType& physic_object_body_type, const bool trigger, const ColliderFilter collider_filter)
{
	const PhysicObjectData& physic_object_data = m_physic_object_data[physic_object_handle];

	float density = 1.0f;
	if (physic_object_body_type == PhysicObjectBodyType::StaticBody || physic_object_body_type == PhysicObjectBodyType::PureStaticBody) //|| physic_object_body_type == PhysicObjectBodyType::KinematicBody)
		density = 0.0f;

	b2Filter filter;
	filter.categoryBits = collider_filter.category_bits;
	filter.maskBits = collider_filter.mask_bits;
	filter.groupIndex = collider_filter.group_index;

	b2Fixture* fixture = physic_object_data.object_body->CreateFixture(physic_object_shape, density);
	fixture->SetSensor(trigger);
	fixture->SetFilterData(filter);
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
	physic_object_data.object_box_fixture = nullptr;
	physic_object_data.object_circle_fixture = nullptr;
	physic_object_data.object_polygon_fixtures.clear();
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

void PhysicsCore::RemovePolygonColliderInternal(const PhysicObjectHandle physic_object_handle)
{
	PhysicObjectData& physic_object_data = m_physic_object_data[physic_object_handle];
	for (auto polygon_fixture : physic_object_data.object_polygon_fixtures)
	{
		physic_object_data.object_body->DestroyFixture(polygon_fixture);
	}
	physic_object_data.object_polygon_fixtures.clear();
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
	if (entity_manager->HasComponent<PureStaticBodyComponent>(entity))
	{
		assert(PhysicObjectBodyType::PureStaticBody == physic_object_body_type);
	}
	if (entity_manager->HasComponent<KinematicBodyComponent>(entity))
	{
		assert(PhysicObjectBodyType::KinematicBody == physic_object_body_type);
	}

	if (!entity_manager->HasComponent<DynamicBodyComponent>(entity) && physic_object_body_type == PhysicObjectBodyType::DynamicBody)
	{
		entity_manager->AddComponent<DynamicBodyComponent>(entity).physic_object_handle = NULL_PHYSIC_OBJECT_HANDLE;
	}
	else if (!entity_manager->HasComponent<StaticBodyComponent>(entity) && physic_object_body_type == PhysicObjectBodyType::StaticBody)
	{
		entity_manager->AddComponent<StaticBodyComponent>(entity).physic_object_handle = NULL_PHYSIC_OBJECT_HANDLE;
	}
	else if (!entity_manager->HasComponent<PureStaticBodyComponent>(entity) && physic_object_body_type == PhysicObjectBodyType::PureStaticBody)
	{
		entity_manager->AddComponent<PureStaticBodyComponent>(entity).physic_object_handle = NULL_PHYSIC_OBJECT_HANDLE;
	}
	else if (!entity_manager->HasComponent<KinematicBodyComponent>(entity) && physic_object_body_type == PhysicObjectBodyType::KinematicBody)
	{
		entity_manager->AddComponent<KinematicBodyComponent>(entity).physic_object_handle = NULL_PHYSIC_OBJECT_HANDLE;
	}

	if (ShouldDeferPhysicCalls(scene_index))
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

	b2BodyType body_type{};
	switch (physic_object_body_type)
	{
	case PhysicObjectBodyType::DynamicBody:
		body_type = b2BodyType::b2_dynamicBody;
		break;
	case PhysicObjectBodyType::StaticBody:
	case PhysicObjectBodyType::PureStaticBody:
		body_type = b2BodyType::b2_staticBody;
		break;
	case PhysicObjectBodyType::KinematicBody:
		body_type = b2BodyType::b2_kinematicBody;
		break;
	}
	physic_body_def.type = body_type;

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
		if (!SceneManager::GetSceneManager()->GetScene(scene_index)->IsSceneActive())
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
		if (!SceneManager::GetSceneManager()->GetScene(scene_index)->IsSceneActive())
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
	else if (physic_object_body_type == PhysicObjectBodyType::PureStaticBody)
	{
		PureStaticBodyComponent& pure_static_body = entity_manager->GetComponent<PureStaticBodyComponent>(entity);
		pure_static_body.physic_object_handle = new_physic_object_handle;
#ifndef _EDITOR
		if (!SceneManager::GetSceneManager()->GetScene(scene_index)->IsSceneActive())
		{
			physic_object_data.object_body->SetEnabled(false);
			//static_body.enabled = physic_object_data.object_body->IsEnabled();
		}
#endif // !_EDITOR
#ifdef _EDITOR
		physic_object_data.object_body->SetEnabled(false);
		physic_object_data.object_body->SetAwake(false);
#endif // _EDITOR
	}
	else if (physic_object_body_type == PhysicObjectBodyType::KinematicBody)
	{
		KinematicBodyComponent& kinematic_body = entity_manager->GetComponent<KinematicBodyComponent>(entity);
		kinematic_body.physic_object_handle = new_physic_object_handle;
#ifndef _EDITOR
		if (!SceneManager::GetSceneManager()->GetScene(scene_index)->IsSceneActive())
		{
			physic_object_data.object_body->SetEnabled(false);
			kinematic_body.enabled = physic_object_data.object_body->IsEnabled();
			physic_object_data.object_body->SetAwake(false);
			kinematic_body.awake = physic_object_data.object_body->IsAwake();
		}
#endif // !_EDITOR
#ifdef _EDITOR
		physic_object_data.object_body->SetEnabled(false);
		kinematic_body.enabled = physic_object_data.object_body->IsEnabled();
		physic_object_data.object_body->SetAwake(false);
		kinematic_body.awake = physic_object_data.object_body->IsAwake();
#endif // _EDITOR
	}
}

void PhysicsCore::AddBoxCollider(const SceneIndex scene_index, const Entity entity, const Vector2& half_box_size, const bool trigger, const ColliderFilter collider_filter)
{
	EntityManager* entity_manager = SceneManager::GetSceneManager()->GetEntityManager(scene_index);

	if (!entity_manager->HasComponent<BoxColliderComponent>(entity))
	{
		BoxColliderComponent& box_collider = entity_manager->AddComponent<BoxColliderComponent>(entity);
		box_collider.physic_object_handle = NULL_PHYSIC_OBJECT_HANDLE;
		box_collider.trigger = trigger;
		box_collider.filter = collider_filter;
		box_collider.half_box_size = half_box_size;
		box_collider.debug_draw = true;
	}

	if (ShouldDeferPhysicCalls(scene_index))
	{
		AddBoxColliderDeferredPhysicObjectCreation(scene_index, entity);
		return;
	}

	AddBoxFixture(scene_index, entity, half_box_size, trigger, collider_filter);
}

void PhysicsCore::AddCircleCollider(const SceneIndex scene_index, const Entity entity, const float circle_radius, const bool trigger, const ColliderFilter collider_filter)
{
	EntityManager* entity_manager = SceneManager::GetSceneManager()->GetEntityManager(scene_index);

	if (!entity_manager->HasComponent<CircleColliderComponent>(entity))
	{
		CircleColliderComponent& circle_collider = entity_manager->AddComponent<CircleColliderComponent>(entity);
		circle_collider.physic_object_handle = NULL_PHYSIC_OBJECT_HANDLE;
		circle_collider.trigger = trigger;
		circle_collider.filter = collider_filter;
		circle_collider.circle_radius = circle_radius;
		circle_collider.debug_draw = true;
	}

	if (ShouldDeferPhysicCalls(scene_index))
	{
		AddCircleColliderDeferredPhysicObjectCreation(scene_index, entity);
		return;
	}

	AddCircleFixture(scene_index, entity, circle_radius, trigger, collider_filter);
}

void PhysicsCore::AddPolygonCollider(const SceneIndex scene_index, const Entity entity, const std::vector<Vector2>& points, bool loop, bool solid, const bool trigger, const ColliderFilter collider_filter)
{
	EntityManager* entity_manager = SceneManager::GetSceneManager()->GetEntityManager(scene_index);

	if (!entity_manager->HasComponent<PolygonColliderComponent>(entity))
	{
		PolygonColliderComponent& polygon_collider = entity_manager->AddComponent<PolygonColliderComponent>(entity);
		polygon_collider.physic_object_handle = NULL_PHYSIC_OBJECT_HANDLE;
		polygon_collider.trigger = trigger;
		polygon_collider.filter = collider_filter;
		polygon_collider.points = points;
		polygon_collider.debug_draw = true;
	}

	if (ShouldDeferPhysicCalls(scene_index))
	{
		AddPolygonColliderDeferredPhysicObjectCreation(scene_index, entity);
		return;
	}

	AddPolygonFixture(scene_index, entity, points, loop, solid, trigger, collider_filter);
}

void PhysicsCore::RemoveBoxCollider(const SceneIndex scene_index, const Entity entity)
{
	EntityManager* entity_manager = SceneManager::GetSceneManager()->GetEntityManager(scene_index);

	if (entity_manager->HasComponent<BoxColliderComponent>(entity))
	{
		PhysicObjectHandle physic_object_handle = entity_manager->GetComponent<BoxColliderComponent>(entity).physic_object_handle;
		entity_manager->RemoveComponent<BoxColliderComponent>(entity);

		if (ShouldDeferPhysicCalls(scene_index))
		{
			AddDeferredPhysicObjectDestruction(scene_index, entity, physic_object_handle, true, ColliderType::Box);
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

		if (ShouldDeferPhysicCalls(scene_index))
		{
			AddDeferredPhysicObjectDestruction(scene_index, entity, physic_object_handle, true, ColliderType::Circle);
			return;
		}

		RemoveCircleColliderInternal(physic_object_handle);
	}
}

void PhysicsCore::RemovePolygonCollider(SceneIndex scene_index, Entity entity)
{
	EntityManager* entity_manager = SceneManager::GetSceneManager()->GetEntityManager(scene_index);

	if (entity_manager->HasComponent<PolygonColliderComponent>(entity))
	{
		PhysicObjectHandle physic_object_handle = entity_manager->GetComponent<PolygonColliderComponent>(entity).physic_object_handle;
		entity_manager->RemoveComponent<PolygonColliderComponent>(entity);

		if (ShouldDeferPhysicCalls(scene_index))
		{
			AddDeferredPhysicObjectDestruction(scene_index, entity, physic_object_handle, true, ColliderType::Polygon);
			return;
		}

		RemovePolygonColliderInternal(physic_object_handle);
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

	entity_manager->System<DeferredEntityDeletion, PureStaticBodyComponent>([&](const Entity entity, DeferredEntityDeletion, PureStaticBodyComponent)
		{
			RemovePhysicObject(entity_manager->GetSceneIndex(), entity);
		});

	entity_manager->System<DeferredEntityDeletion, KinematicBodyComponent>([&](const Entity entity, DeferredEntityDeletion, KinematicBodyComponent)
		{
			RemovePhysicObject(entity_manager->GetSceneIndex(), entity);
		});
}

class PhysicsRaycastCallback : public b2RayCastCallback
{
public:
	PhysicsRaycastCallback(const ColliderFilter collider_filter, const std::function<bool(bool, float, float, SceneIndex, Entity)>& raycast_logic)
		: m_collider_filter(collider_filter), m_raycast_logic(raycast_logic)
	{
		m_closest_result.intersected = false;
	}

	RaycastResult GetResult() const
	{
		return m_closest_result;
	}

private:
	float ReportFixture(b2Fixture* fixture, const b2Vec2& point,
		const b2Vec2& normal, float fraction)
	{
		const bool should_collide = PhysicsContactFilter::ShouldCollide(fixture, m_collider_filter);
		const bool should_raycast = should_collide && !fixture->IsSensor();
		const auto entity_data = PhysicsCore::Get()->GetEntityAndSceneFromUserData((void*)fixture->GetBody()->GetUserData().pointer);

		if (m_raycast_logic(should_raycast, fraction, m_closest_fraction, entity_data.second, entity_data.first)) {
			m_closest_fraction = fraction;
			m_closest_result.position = Vector2(point.x, point.y);
			m_closest_result.entity = entity_data.first;
			m_closest_result.scene_index = entity_data.second;

			m_closest_result.intersected = true;
		}

		return 1.0f;
	}

private:
	ColliderFilter m_collider_filter;
	RaycastResult m_closest_result;
	float m_closest_fraction = 9999;
	std::function<bool(bool, float, float, SceneIndex, Entity)> m_raycast_logic;
};

RaycastResult PhysicsCore::Raycast(const Vector2& position, const Vector2& direction, const ColliderFilter collider_filter, 
	const std::function<bool(bool, float, float, SceneIndex, Entity)>& raycast_logic)
{
	const b2Vec2 b2_position(position.x, position.y);
	PhysicsRaycastCallback callback(collider_filter, raycast_logic);
	m_world->RayCast(&callback, b2_position, b2_position + b2Vec2(direction.x * 100.0f, direction.y * 100.0f));
	return callback.GetResult();
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
	else if (entity_manager->HasComponent<PureStaticBodyComponent>(entity))
	{
		physic_object_handle = entity_manager->GetComponent<PureStaticBodyComponent>(entity).physic_object_handle;
	}
	else if (entity_manager->HasComponent<KinematicBodyComponent>(entity))
	{
		physic_object_handle = entity_manager->GetComponent<KinematicBodyComponent>(entity).physic_object_handle;
	}

	assert(physic_object_handle != NULL_PHYSIC_OBJECT_HANDLE);

	return physic_object_handle;
}

std::pair<Entity, SceneIndex> PhysicsCore::GetEntityAndSceneFromUserData(void* user_data) const
{
	PhysicObjectData* physic_object_data = (PhysicObjectData*)(user_data);
	return std::pair<Entity, SceneIndex>(physic_object_data->object_entity, physic_object_data->object_scene_index);
}
