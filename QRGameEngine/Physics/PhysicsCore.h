#pragma once
#include "ECS/EntityDefinition.h"
#include "PhysicDefines.h"
#include "Common/EngineTypes.h"
#include "SceneSystem/SceneDefines.h"
#include <thread>
#include <mutex>

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
	friend PhysicsContactListener;

public:
	enum PhysicObjectBodyType
	{
		StaticBody = 0,
		KinematicBody,
		DynamicBody,
		PureStaticBody = 5,
		EmptyBody = 10
	};

private:
	struct PhysicObjectData {
		b2Body* object_body;
		PhysicObjectBodyType object_body_type;
		Entity object_entity = NULL_ENTITY;
		SceneIndex object_scene_index;

		b2Fixture* object_box_fixture = nullptr;
		b2Fixture* object_circle_fixture = nullptr;
		std::vector<b2Fixture*> object_polygon_fixtures;
	};

	struct DeferredPhysicObjectHandle
	{
		bool is_physic_object_creation_data;
		uint64_t index_to_physic_object_data;
	};

	enum class ColliderType
	{
		Box,
		Circle,
		Polygon,
		None
	};

	struct DeferredPhysicObjectCreationData
	{
		Entity entity;
		SceneIndex scene_index;
		PhysicObjectBodyType physic_object_body_type;

		bool has_collider_data;
		ColliderType collider_type;
	};

	struct DeferredPhysicObjectDestructionData
	{
		Entity entity;
		SceneIndex scene_index;
		bool is_collider;
		ColliderType collider_type;
		PhysicObjectHandle physic_object_handle;
	};

	enum class PhysicThreadState
	{
		Wait = 0,
		Update = 1,
		Close = 2
	};

private:
	b2World* m_world;
	PhysicsDebugDraw* m_debug_draw;
	PhysicsContactListener* m_contact_listener;
	PhysicsDestructionListener* m_destruction_listener;

	static PhysicsCore* s_physics_core;

	static constexpr uint64_t MAX_PHYSIC_OBJECTS = 2000 * 4;
	std::vector<PhysicObjectData> m_physic_object_data;
	std::stack<PhysicObjectHandle> m_free_physic_object_handles;

	static constexpr float TIME_STEP = 1.0f / 120.0f;
	static constexpr int32_t VELOCITY_ITERATIONS = 8;
	static constexpr int32_t POSITION_ITERATIONS = 3;
	float m_time_since_last_update = 0.0f;

	bool m_threaded_physics;
	std::thread* m_physic_update_thread = nullptr;
	std::mutex m_physic_update_thread_mutex;
	std::atomic<PhysicThreadState> m_update_physics;
	bool m_defer_physic_calls;

	std::vector<DeferredPhysicObjectHandle> m_deferred_physic_object_handles;
	std::vector<DeferredPhysicObjectCreationData> m_deferred_physic_object_creations;
	std::vector<DeferredPhysicObjectDestructionData> m_deferred_physic_object_destructions;

	struct DeferredEnablePureStaticBody
	{
		SceneIndex scene_index;
		Entity entity;
	};
	std::vector<DeferredEnablePureStaticBody> m_deferred_enable_pure_static_bodies;

private:
	b2Fixture* AddFixtureToPhysicObject(PhysicObjectHandle physic_object_handle, b2Shape* physic_object_shape, const PhysicObjectBodyType& physic_object_body_type, bool trigger, ColliderFilter collider_filter);

	void RemovePhysicObjectInternal(PhysicObjectHandle physic_object_handle);
	void RemoveBoxColliderInternal(PhysicObjectHandle physic_object_handle);
	void RemoveCircleColliderInternal(PhysicObjectHandle physic_object_handle);
	void RemovePolygonColliderInternal(PhysicObjectHandle physic_object_handle);

	PhysicObjectHandle GetPhysicObjectHandle(EntityManager* entity_manager, Entity entity);

	std::pair<Entity, SceneIndex> GetEntityAndSceneFromUserData(void* user_data) const;

	bool IsDeferringPhysicCalls();

	void AddDeferredPhysicObjectHandle(uint64_t index_to_data, bool is_physic_object_creation_data);

	void AddDeferredPhysicObjectCreation(SceneIndex scene_index, Entity entity, const PhysicObjectBodyType& physic_object_body_type);
	void AddBoxColliderDeferredPhysicObjectCreation(SceneIndex scene_index, Entity entity);
	void AddCircleColliderDeferredPhysicObjectCreation(SceneIndex scene_index, Entity entity);
	void AddPolygonColliderDeferredPhysicObjectCreation(SceneIndex scene_index, Entity entity);

	void AddDeferredPhysicObjectDestruction(const SceneIndex scene_index, const Entity entity, const PhysicObjectHandle physic_object_handle, const bool is_collider, const ColliderType collider_type);

	void HandleDeferredPhysicObjectHandleData();
	void HandleDeferredPhysicObjectCreationData(const DeferredPhysicObjectCreationData& creation_data);
	void HandleDeferredPhysicObjectDestructionData(const DeferredPhysicObjectDestructionData& destruction_data);

	void AddBoxFixture(SceneIndex scene_index, Entity entity, const Vector2& half_box_size, bool trigger = false, ColliderFilter collider_filter = {});
	void AddCircleFixture(SceneIndex scene_index, Entity entity, float circle_radius, bool trigger = false, ColliderFilter collider_filter = {});
	void AddPolygonFixture(SceneIndex scene_index, Entity entity, const std::vector<Vector2>& points, bool trigger = false, ColliderFilter collider_filter = {});

	static void AwakePhysicObjectsFromLoadedScene(SceneIndex scene_index);

	void AwakePureStaticBody(SceneIndex scene_index, Entity entity);

public:
	PhysicsCore(bool threaded_physics);
	~PhysicsCore();

	static PhysicsCore* Get();

	const bool& IsThreaded() const;

	void testg();

	void WaitForPhysics();

	void ThreadUpdatePhysic();

	void Update();

	void UpdatePhysics();

	void DrawColliders();
	void HandleDeferredPhysicData();
	void HandleDeferredCollisionData();

	void SetWorldPhysicObjectData(EntityManager* entity_manager);
	void GetWorldPhysicObjectData(EntityManager* entity_manager);

	void AddPhysicObject(SceneIndex scene_index, Entity entity, const PhysicObjectBodyType& physic_object_body_type);
	void AddBoxCollider(SceneIndex scene_index, Entity entity, const Vector2& half_box_size, bool trigger = false, ColliderFilter collider_filter = {});
	void AddCircleCollider(SceneIndex scene_index, Entity entity, float circle_radius, bool trigger = false, ColliderFilter collider_filter = {});
	void AddPolygonCollider(SceneIndex scene_index, Entity entity, const std::vector<Vector2>& points, bool trigger = false, ColliderFilter collider_filter = {});
	void AddBoxPhysicObject(SceneIndex scene_index, Entity entity, const PhysicObjectBodyType& physic_object_body_type, const Vector2& half_box_size, bool trigger = false, ColliderFilter collider_filter = {});
	void AddCirclePhysicObject(SceneIndex scene_index, Entity entity, const PhysicObjectBodyType& physic_object_body_type, float circle_radius, bool trigger = false, ColliderFilter collider_filter = {});

	void RemovePhysicObject(SceneIndex scene_index, Entity entity);
	void RemoveBoxCollider(SceneIndex scene_index, Entity entity);
	void RemoveCircleCollider(SceneIndex scene_index, Entity entity);
	void RemovePolygonCollider(SceneIndex scene_index, Entity entity);

	void RemoveDeferredPhysicObjects(EntityManager* entity_manager);
};

