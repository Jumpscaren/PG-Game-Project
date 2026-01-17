#include "pch.h"
#include "PhysicsCore.h"

#include "Vendor/Include/Box2D/IncludeBox2D.h"
#include "Renderer/RenderCore.h"
#include "PhysicsInternal/PhysicsDebugDraw.h"
#include "PhysicsInternal/PhysicsContactFilter.h"
#include "PhysicsInternal/PhysicsContactListener.h"

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

	b2Vec2 gravity;
	gravity = b2Vec2{ 0.0f, 0.0f };

	b2WorldDef world_def = b2DefaultWorldDef();
	world_def.gravity = gravity;

	m_world = b2CreateWorld(&world_def);

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
	b2DestroyWorld(m_world);
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

	const b2Polygon shape = b2MakeBox(half_box_size.x * scale.x, half_box_size.y * scale.y);

	PhysicObjectHandle physic_object_handle = GetPhysicObjectHandle(entity_manager, entity);

	PhysicObjectData& physic_object_data = m_physic_object_data[physic_object_handle];

	const b2ShapeDef shape_def = CreateShapeDef(physic_object_handle, physic_object_data.object_body_type, trigger, collider_filter);
	physic_object_data.object_box_shape = b2CreatePolygonShape(physic_object_data.object_body, &shape_def, &shape);

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
	const Vector2 position = transform.GetPosition2D();

	b2Circle shape;
	shape.radius = circle_radius * scale.x;
	shape.center = b2Vec2_zero;

	PhysicObjectHandle physic_object_handle = GetPhysicObjectHandle(entity_manager, entity);

	PhysicObjectData& physic_object_data = m_physic_object_data[physic_object_handle];

	const b2ShapeDef shape_def = CreateShapeDef(physic_object_handle, physic_object_data.object_body_type, trigger, collider_filter);
	physic_object_data.object_circle_shape = b2CreateCircleShape(physic_object_data.object_body, &shape_def, &shape);

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
	if (points.size() < 4)
	{
		const b2Vec2 fake_point = b2Vec2(100.0f, 100.0f);
		b2_points.push_back(fake_point);
	}

	b2ChainDef shape = b2DefaultChainDef();
	shape.points = b2_points.data();
	shape.count = (int)b2_points.size();
	shape.isLoop = loop;

	const b2ShapeDef shape_def = CreateShapeDef(physic_object_handle, physic_object_data.object_body_type, trigger, collider_filter);
	shape.enableSensorEvents = shape_def.enableSensorEvents;
	shape.filter = shape_def.filter;

	physic_object_data.object_chain_shape = b2CreateChain(physic_object_data.object_body, &shape);
#else
	if (loop && solid)
	{
		// Could be included in the component data, but we don't need it right now
		constexpr float RADIUS = 0.0f;

		const auto triangles = PolygonColliderComponentInterface::CreatePolygonTriangulation(entity, entity_manager);
		for (const auto& triangle : triangles)
		{
			std::vector<b2Vec2> b2_points;
			b2_points.push_back(b2Vec2(triangle.prev_point.x, triangle.prev_point.y));
			b2_points.push_back(b2Vec2(triangle.point.x, triangle.point.y));
			b2_points.push_back(b2Vec2(triangle.next_point.x, triangle.next_point.y));

			const b2Hull hull = b2ComputeHull(b2_points.data(), (int)b2_points.size());
			const b2Polygon shape = b2MakePolygon(&hull, RADIUS);

			const b2ShapeDef shape_def = CreateShapeDef(physic_object_handle, physic_object_data.object_body_type, trigger, collider_filter);

			physic_object_data.object_polygon_shapes.push_back(b2CreatePolygonShape(physic_object_data.object_body, &shape_def, &shape));
		}
	}
	else
	{
		std::vector<b2Vec2> b2_points;
		for (const auto& point : points)
		{
			b2_points.push_back(b2Vec2(point.x, point.y));
		}

		b2ChainDef shape = b2DefaultChainDef();
		shape.points = b2_points.data();
		shape.count = (int)b2_points.size();
		shape.isLoop = loop;

		const b2ShapeDef shape_def = CreateShapeDef(physic_object_handle, physic_object_data.object_body_type, trigger, collider_filter);
		shape.enableSensorEvents = shape_def.enableSensorEvents;
		shape.filter = shape_def.filter;

		physic_object_data.object_chain_shape = b2CreateChain(physic_object_data.object_body, &shape);
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
		b2World_Step(m_world, TIME_STEP, SUB_STEP_COUNT);
		m_contact_listener->HandleContacts();
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
	std::vector<Vector2> vertices;
	entity_manager->System<TransformComponent, BoxColliderComponent>([&](const Entity entity, const TransformComponent& transform, const BoxColliderComponent& box_collider)
		{
			if (!box_collider.debug_draw)
			{
				return;
			}

			const Vector2 position = transform.GetPosition2D() + box_collider.offset;

			const auto rotation_z = transform.GetRotationEuler().z;
			const Vector3& scale = transform.GetScale();

			const Vector2 top_left_corner = Vector2(-box_collider.half_box_size.x * scale.x, box_collider.half_box_size.y * scale.y);
			const Vector2 top_right_corner = Vector2(box_collider.half_box_size.x * scale.x, box_collider.half_box_size.y * scale.y);
			const Vector2 bottom_left_corner = Vector2(-box_collider.half_box_size.x * scale.x, -box_collider.half_box_size.y * scale.y);
			const Vector2 bottom_right_corner = Vector2(box_collider.half_box_size.x * scale.x, -box_collider.half_box_size.y * scale.y);

			const Vector2 rotated_top_left_corner = Vector2(top_left_corner.x * cosf(rotation_z) - top_left_corner.y * sinf(rotation_z), top_left_corner.x * sinf(rotation_z) + top_left_corner.y * cosf(rotation_z));
			const Vector2 rotated_top_right_corner = Vector2(top_right_corner.x * cosf(rotation_z) - top_right_corner.y * sinf(rotation_z), top_right_corner.x * sinf(rotation_z) + top_right_corner.y * cosf(rotation_z));
			const Vector2 rotated_bottom_left_corner = Vector2(bottom_left_corner.x * cosf(rotation_z) - bottom_left_corner.y * sinf(rotation_z), bottom_left_corner.x * sinf(rotation_z) + bottom_left_corner.y * cosf(rotation_z));
			const Vector2 rotated_bottom_right_corner = Vector2(bottom_right_corner.x * cosf(rotation_z) - bottom_right_corner.y * sinf(rotation_z), bottom_right_corner.x * sinf(rotation_z) + bottom_right_corner.y * cosf(rotation_z));

			const Vector2 offseted_position = GetOffsetedPosition(entity_manager, entity, transform);

			const Vector2 physic_position(offseted_position.x, offseted_position.y);
			vertices.push_back(physic_position + rotated_top_left_corner);
			vertices.push_back(physic_position + rotated_bottom_left_corner);
			vertices.push_back(physic_position + rotated_bottom_right_corner);
			vertices.push_back(physic_position + rotated_top_right_corner);

			m_debug_draw->DrawPolygon(vertices, Vector3(1.0f, 0.0f, 0.0f));

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

			const Vector2 rotated_circle_pointer = Vector2(circle_collider.circle_radius * cosf(rotation_z), circle_collider.circle_radius * sinf(rotation_z));

			vertices.push_back(Vector2(position.x, position.y));
			vertices.push_back(Vector2(position.x, position.y) + rotated_circle_pointer);

			m_debug_draw->DrawPolygon(vertices, Vector3(1.0f, 0.0f, 0.0f));
			m_debug_draw->DrawCircle(Vector2(position.x, position.y), circle_collider.circle_radius, Vector3(1.0f, 0.0f, 0.0f));

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

			const Vector2 position_2d(position.x, position.y);

			for (int i = 0; i < polygon_collider.points.size() - 1; ++i)
			{
				const Vector2 point_1 = polygon_collider.points[i];
				const Vector2 point_2 = polygon_collider.points[i + 1];

				m_debug_draw->DrawSegment(position_2d + Vector2(point_1.x, point_1.y), position_2d + Vector2(point_2.x, point_2.y), Vector3(1.0f, 0.0f, 0.0f));
			}

			if (polygon_collider.loop)
			{
				const Vector2 point_1 = polygon_collider.points.back();
				const Vector2 point_2 = polygon_collider.points.front();
				m_debug_draw->DrawSegment(position_2d + Vector2(point_1.x, point_1.y), position_2d + Vector2(point_2.x, point_2.y), Vector3(1.0f, 0.0f, 0.0f));
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
	entity_manager->System<DynamicBodyComponent, TransformComponent>([&](const DynamicBodyComponent& dynamic_body, const TransformComponent& transform)
		{
			const PhysicThreadState threat_run = m_update_physics;
			PhysicObjectData& physic_object = m_physic_object_data[dynamic_body.physic_object_handle];

			b2Body_SetTransform(physic_object.object_body, b2Vec2(transform.GetPosition().x, transform.GetPosition().y), b2MakeRot(transform.GetRotationEuler().z));
			if (dynamic_body.awake != b2Body_IsAwake(physic_object.object_body))
			{
				b2Body_SetAwake(physic_object.object_body, dynamic_body.awake);
			}
			b2Body_SetLinearVelocity(physic_object.object_body, b2Vec2(dynamic_body.velocity.x, dynamic_body.velocity.y));

			if (dynamic_body.fixed_rotation != b2Body_GetMotionLocks(physic_object.object_body).angularZ)
			{
				b2Body_SetMotionLocks(physic_object.object_body, b2MotionLocks{ .linearX = false, .linearY = false, .angularZ = dynamic_body.fixed_rotation });
			}

			const bool enabled = b2Body_IsEnabled(physic_object.object_body);
			if (dynamic_body.enabled != enabled)
			{
				if (dynamic_body.enabled)
				{
					b2Body_Enable(physic_object.object_body);
				}
				else
				{
					b2Body_Disable(physic_object.object_body);
				}
			}
		});

	entity_manager->System<StaticBodyComponent, TransformComponent>([&](const Entity entity, const StaticBodyComponent& static_body, const TransformComponent& transform)
		{
			PhysicObjectData& physic_object = m_physic_object_data[static_body.physic_object_handle];

			const float rotation_z = transform.GetRotationEuler().z;

			const Vector2 position = GetOffsetedPosition(entity_manager, entity, transform);

			b2Body_SetTransform(physic_object.object_body, b2Vec2(transform.GetPosition().x, transform.GetPosition().y), b2MakeRot(transform.GetRotationEuler().z));

			const bool enabled = b2Body_IsEnabled(physic_object.object_body);
			if (static_body.enabled != enabled)
			{
				if (static_body.enabled)
				{
					b2Body_Enable(physic_object.object_body);
				}
				else
				{
					b2Body_Disable(physic_object.object_body);
				}
			}
		});

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
			b2Body_SetTransform(physic_object.object_body, b2Vec2(position.x, position.y), b2MakeRot(transform.GetRotationEuler().z));
			b2Body_Enable(physic_object.object_body);
			b2Body_SetAwake(physic_object.object_body, true);
		}
	}
	m_deferred_enable_pure_static_bodies.clear();

	entity_manager->System<KinematicBodyComponent, TransformComponent>([&](const KinematicBodyComponent& kinematic_body, const TransformComponent& transform)
		{
			PhysicObjectData& physic_object = m_physic_object_data[kinematic_body.physic_object_handle];
			b2Body_SetTransform(physic_object.object_body, b2Vec2(transform.GetPosition().x, transform.GetPosition().y), b2MakeRot(transform.GetRotationEuler().z));
			if (kinematic_body.awake != b2Body_IsAwake(physic_object.object_body))
			{
				b2Body_SetAwake(physic_object.object_body, kinematic_body.awake);
			}
			b2Body_SetLinearVelocity(physic_object.object_body, b2Vec2(kinematic_body.velocity.x, kinematic_body.velocity.y));

			if (kinematic_body.fixed_rotation != b2Body_GetMotionLocks(physic_object.object_body).angularZ)
			{
				b2Body_SetMotionLocks(physic_object.object_body, b2MotionLocks{ .linearX = false, .linearY = false, .angularZ = kinematic_body.fixed_rotation });
			}

			const bool enabled = b2Body_IsEnabled(physic_object.object_body);
			if (kinematic_body.enabled != enabled)
			{
				if (kinematic_body.enabled)
				{
					b2Body_Enable(physic_object.object_body);
				}
				else
				{
					b2Body_Disable(physic_object.object_body);
				}
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
			 const b2Transform transform_physic_object = b2Body_GetTransform(physic_object.object_body);
			 transform.SetPosition(Vector2(transform_physic_object.p.x, transform_physic_object.p.y));

			 Vector3 rotation = transform.GetRotationEuler();
			 rotation.z = b2Rot_GetAngle(transform_physic_object.q);
			 transform.SetRotation(rotation);

			 const b2Vec2 velocity = b2Body_GetLinearVelocity(physic_object.object_body);
			 dynamic_body.velocity.x = velocity.x;
			 dynamic_body.velocity.y = velocity.y;
		});

	entity_manager->System<KinematicBodyComponent, TransformComponent>([&](KinematicBodyComponent& kinematic_body, TransformComponent& transform)
		{
			PhysicObjectData& physic_object = m_physic_object_data[kinematic_body.physic_object_handle];
			const b2Transform transform_physic_object = b2Body_GetTransform(physic_object.object_body);
			transform.SetPosition(Vector2(transform_physic_object.p.x, transform_physic_object.p.y));

			Vector3 rotation = transform.GetRotationEuler();
			rotation.z = b2Rot_GetAngle(transform_physic_object.q);
			transform.SetRotation(rotation);

			const b2Vec2 velocity = b2Body_GetLinearVelocity(physic_object.object_body);
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

b2ShapeDef PhysicsCore::CreateShapeDef(const PhysicObjectHandle physic_object_handle, const PhysicObjectBodyType& physic_object_body_type, const bool trigger, const ColliderFilter collider_filter)
{
	const PhysicObjectData& physic_object_data = m_physic_object_data[physic_object_handle];

	float density = 1.0f;
	if (physic_object_body_type == PhysicObjectBodyType::StaticBody || physic_object_body_type == PhysicObjectBodyType::PureStaticBody) //|| physic_object_body_type == PhysicObjectBodyType::KinematicBody)
		density = 0.0f;

	b2ShapeDef shape_def = b2DefaultShapeDef();
	shape_def.density = density;
	shape_def.material.friction = 0.0f;
	shape_def.material.restitution = 0.0f;
	shape_def.isSensor = trigger;

	shape_def.filter.categoryBits = collider_filter.category_bits;
	shape_def.filter.maskBits = collider_filter.mask_bits;
	shape_def.filter.groupIndex = collider_filter.group_index;
	shape_def.enableCustomFiltering = true;

	if (physic_object_body_type == PhysicObjectBodyType::DynamicBody || physic_object_body_type == PhysicObjectBodyType::KinematicBody)
	{
		shape_def.enableContactEvents = true;
		shape_def.enableSensorEvents = true;
		//shape_def.enableHitEvents = true;
	}
	else
	{
		shape_def.enableSensorEvents = trigger;
	}

	return shape_def;
}

void PhysicsCore::RemovePhysicObjectInternal(const PhysicObjectHandle physic_object_handle)
{
	assert(physic_object_handle != NULL_PHYSIC_OBJECT_HANDLE);
	m_free_physic_object_handles.push(physic_object_handle);

	PhysicObjectData& physic_object_data = m_physic_object_data[physic_object_handle];
	b2DestroyBody(physic_object_data.object_body);

	physic_object_data.object_body = b2_nullBodyId;
	physic_object_data.object_body_type = PhysicObjectBodyType::EmptyBody;
	physic_object_data.object_entity = NULL_ENTITY;
	physic_object_data.object_box_shape = b2_nullShapeId;
	physic_object_data.object_circle_shape = b2_nullShapeId;
	physic_object_data.object_polygon_shapes.clear();
	physic_object_data.object_chain_shape = b2_nullChainId;
}

void PhysicsCore::RemoveBoxColliderInternal(const PhysicObjectHandle physic_object_handle)
{
	PhysicObjectData& physic_object_data = m_physic_object_data[physic_object_handle];
	b2DestroyShape(physic_object_data.object_box_shape, UPDATE_BODY_MASS_WHEN_DESTROYING_SHAPE);
	physic_object_data.object_box_shape = b2_nullShapeId;
}

void PhysicsCore::RemoveCircleColliderInternal(const PhysicObjectHandle physic_object_handle)
{
	PhysicObjectData& physic_object_data = m_physic_object_data[physic_object_handle];
	b2DestroyShape(physic_object_data.object_circle_shape, UPDATE_BODY_MASS_WHEN_DESTROYING_SHAPE);
	physic_object_data.object_circle_shape = b2_nullShapeId;
}

void PhysicsCore::RemovePolygonColliderInternal(const PhysicObjectHandle physic_object_handle)
{
	PhysicObjectData& physic_object_data = m_physic_object_data[physic_object_handle];
	for (const b2ShapeId polygon_shape : physic_object_data.object_polygon_shapes)
	{
		b2DestroyShape(polygon_shape, UPDATE_BODY_MASS_WHEN_DESTROYING_SHAPE);
	}
	physic_object_data.object_polygon_shapes.clear();
	
	if (B2_IS_NON_NULL(physic_object_data.object_chain_shape))
	{
		b2DestroyChain(physic_object_data.object_chain_shape);
		physic_object_data.object_chain_shape = b2_nullChainId;
	}
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

	b2BodyDef physic_body_def = b2DefaultBodyDef();
	physic_body_def.position = physic_object_position;
	physic_body_def.isAwake = true;
	physic_body_def.enableSleep = true;
	physic_body_def.isEnabled = true;

#ifndef _EDITOR
	if (!SceneManager::GetSceneManager()->GetScene(scene_index)->IsSceneActive())
	{
		physic_body_def.isEnabled = false;
		physic_body_def.isAwake = false;
	}
#endif // !_EDITOR
#ifdef _EDITOR
	physic_body_def.isEnabled = false;
	physic_body_def.isAwake = false;
#endif // _EDITOR

	b2BodyType body_type{};
	switch (physic_object_body_type)
	{
	case PhysicObjectBodyType::DynamicBody:
		body_type = b2BodyType::b2_dynamicBody;
		break;
	case PhysicObjectBodyType::StaticBody:
		body_type = b2BodyType::b2_staticBody;
		break;
	case PhysicObjectBodyType::PureStaticBody:
		body_type = b2BodyType::b2_staticBody;
		break;
	case PhysicObjectBodyType::KinematicBody:
		body_type = b2BodyType::b2_kinematicBody;
		break;
	}
	physic_body_def.type = body_type;

	physic_body_def.userData = (void*)&m_physic_object_data[new_physic_object_handle];

	PhysicObjectData& physic_object_data = m_physic_object_data[new_physic_object_handle];
	physic_object_data.object_body = b2CreateBody(m_world, &physic_body_def);
	physic_object_data.object_body_type = physic_object_body_type;
	physic_object_data.object_entity = entity;
	physic_object_data.object_scene_index = scene_index;

	if (physic_object_body_type == PhysicObjectBodyType::DynamicBody)
	{
		DynamicBodyComponent& dynamic_body = entity_manager->GetComponent<DynamicBodyComponent>(entity);
		dynamic_body.physic_object_handle = new_physic_object_handle;
	}
	else if (physic_object_body_type == PhysicObjectBodyType::StaticBody)
	{
		StaticBodyComponent& static_body = entity_manager->GetComponent<StaticBodyComponent>(entity);
		static_body.physic_object_handle = new_physic_object_handle;
	}
	else if (physic_object_body_type == PhysicObjectBodyType::PureStaticBody)
	{
		PureStaticBodyComponent& pure_static_body = entity_manager->GetComponent<PureStaticBodyComponent>(entity);
		pure_static_body.physic_object_handle = new_physic_object_handle;
	}
	else if (physic_object_body_type == PhysicObjectBodyType::KinematicBody)
	{
		KinematicBodyComponent& kinematic_body = entity_manager->GetComponent<KinematicBodyComponent>(entity);
		kinematic_body.physic_object_handle = new_physic_object_handle;
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
	entity_manager->System<DeferredEntityDeletion, DynamicBodyComponent>([&](const Entity entity, DeferredEntityDeletion, const DynamicBodyComponent&)
		{
			RemovePhysicObject(entity_manager->GetSceneIndex(), entity);
			m_contact_listener->DeletedEntity(entity_manager->GetSceneIndex(), entity);
		});

	entity_manager->System<DeferredEntityDeletion, StaticBodyComponent>([&](const Entity entity, DeferredEntityDeletion, const StaticBodyComponent&)
		{
			RemovePhysicObject(entity_manager->GetSceneIndex(), entity);
			m_contact_listener->DeletedEntity(entity_manager->GetSceneIndex(), entity);
		});

	entity_manager->System<DeferredEntityDeletion, PureStaticBodyComponent>([&](const Entity entity, DeferredEntityDeletion, const PureStaticBodyComponent&)
		{
			RemovePhysicObject(entity_manager->GetSceneIndex(), entity);
			m_contact_listener->DeletedEntity(entity_manager->GetSceneIndex(), entity);
		});

	entity_manager->System<DeferredEntityDeletion, KinematicBodyComponent>([&](const Entity entity, DeferredEntityDeletion, const KinematicBodyComponent&)
		{
			RemovePhysicObject(entity_manager->GetSceneIndex(), entity);
			m_contact_listener->DeletedEntity(entity_manager->GetSceneIndex(), entity);
		});
}

class PhysicsRaycastCallback
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

	float ReportShape(const b2ShapeId shape_id, const b2Vec2& point,
		const b2Vec2& normal, float fraction)
	{
		const bool should_raycast = !b2Shape_IsSensor(shape_id);
		const auto entity_data = PhysicsCore::Get()->GetEntityAndSceneFromUserData(b2Body_GetUserData(b2Shape_GetBody(shape_id)));

		if (fraction > MIN_FRACTION && m_raycast_logic(should_raycast, fraction, m_closest_fraction, entity_data.second, entity_data.first)) {
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

	static constexpr float MIN_FRACTION = 0.0001f;
};

float RaycastCallback(const b2ShapeId shape_id, const b2Vec2 point, const b2Vec2 normal, const float fraction, void* context)
{
	PhysicsRaycastCallback* physics_raycast_callback = (PhysicsRaycastCallback*)context;
	return physics_raycast_callback->ReportShape(shape_id, point, normal, fraction);
}

RaycastResult PhysicsCore::Raycast(const Vector2& position, const Vector2& direction, const ColliderFilter collider_filter, 
	const std::function<bool(bool, float, float, SceneIndex, Entity)>& raycast_logic)
{
	const b2Vec2 b2_position(position.x, position.y);
	PhysicsRaycastCallback callback(collider_filter, raycast_logic);

	const b2QueryFilter filter{ .categoryBits = collider_filter.category_bits, .maskBits = collider_filter.mask_bits };

	std::ignore = b2World_CastRay(m_world, b2_position, b2_position + b2Vec2(direction.x * 100.0f, direction.y * 100.0f), filter, RaycastCallback, &callback);
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
