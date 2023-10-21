#pragma once
#include "ECS/EntityDefinition.h"
#include "PhysicDefines.h"
#include "Common/EngineTypes.h"
#include "SceneSystem/SceneDefines.h"
#include <thread>

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
	};

	struct DeferredPhysicObjectHandle
	{
		bool is_physic_object_creation_data;
		uint64_t index_to_physic_object_data;
	};

	struct DeferredPhysicObjectCreationData
	{
		Entity entity;
		SceneIndex scene_index;
		PhysicObjectBodyType physic_object_body_type;

		bool has_collider_data;
		bool is_box_collider;
		Vector2 half_box_size;
		float circle_radius;

		bool trigger;
	};

	struct DeferredPhysicObjectDestructionData
	{
		Entity entity;
		SceneIndex scene_index;
		bool is_collider;
		bool is_box_collider;
		PhysicObjectHandle physic_object_handle;
	};

private:
	b2World* m_world;
	PhysicsDebugDraw* m_debug_draw;
	PhysicsContactListener* m_contact_listener;
	PhysicsDestructionListener* m_destruction_listener;

	static PhysicsCore* s_physics_core;

	static constexpr uint64_t MAX_PHYSIC_OBJECTS = 2000;
	std::vector<PhysicObjectData> m_physic_object_data;
	std::stack<PhysicObjectHandle> m_free_physic_object_handles;

	static constexpr float TIME_STEP = 1.0f / 120.0f;
	static constexpr int32_t VELOCITY_ITERATIONS = 8;
	static constexpr int32_t POSITION_ITERATIONS = 3;
	float m_time_since_last_update = 0.0f;

	bool m_threaded_physics;
	std::thread* m_physic_update_thread = nullptr;
	std::mutex m_physic_update_thread_mutex;
	std::atomic<bool> m_update_physics;
	bool m_defer_physic_calls;

	std::vector<DeferredPhysicObjectHandle> m_deferred_physic_object_handles;
	std::vector<DeferredPhysicObjectCreationData> m_deferred_physic_object_creations;
	std::vector<DeferredPhysicObjectDestructionData> m_deferred_physic_object_destructions;

private:
	b2Fixture* AddFixtureToPhysicObject(PhysicObjectHandle physic_object_handle, b2Shape* physic_object_shape, const PhysicObjectBodyType& physic_object_body_type, bool trigger);

	void RemovePhysicObjectInternal(PhysicObjectHandle physic_object_handle);
	void RemoveBoxColliderInternal(PhysicObjectHandle physic_object_handle);
	void RemoveCircleColliderInternal(PhysicObjectHandle physic_object_handle);

	PhysicObjectHandle GetPhysicObjectHandle(EntityManager* entity_manager, Entity entity);

	std::pair<Entity, SceneIndex> GetEntityAndSceneFromUserData(void* user_data) const;

	bool IsDeferringPhysicCalls();

	void AddDeferredPhysicObjectHandle(uint64_t index_to_data, bool is_physic_object_creation_data);

	void AddDeferredPhysicObjectCreation(SceneIndex scene_index, Entity entity, const PhysicObjectBodyType& physic_object_body_type);
	void AddBoxColliderDeferredPhysicObjectCreation(SceneIndex scene_index, Entity entity, const Vector2& half_box_size, bool trigger);
	void AddCircleColliderDeferredPhysicObjectCreation(SceneIndex scene_index, Entity entity, float circle_radius, bool trigger);

	void AddDeferredPhysicObjectDestruction(SceneIndex scene_index, Entity entity, PhysicObjectHandle physic_object_handle, bool is_collider, bool is_box_collider);

	void HandleDeferredPhysicObjectHandleData();
	void HandleDeferredPhysicObjectCreationData(const DeferredPhysicObjectCreationData& creation_data);
	void HandleDeferredPhysicObjectDestructionData(const DeferredPhysicObjectDestructionData& destruction_data);

	void AddBoxFixture(SceneIndex scene_index, Entity entity, const Vector2& half_box_size, bool trigger = false);
	void AddCircleFixture(SceneIndex scene_index, Entity entity, float circle_radius, bool trigger = false);

	static void AwakePhysicObjectsFromLoadedScene(SceneIndex scene_index);

public:
	PhysicsCore(bool threaded_physics);

	static PhysicsCore* Get();

	const bool& IsThreaded() const;

	void testg();

	void WaitForPhysics();

	void ThreadUpdatePhysic();

	void Update();

	void UpdatePhysics();

	void DrawColliders();
	void HandleDeferredPhysicData();

	void SetWorldPhysicObjectData(EntityManager* entity_manager);
	void GetWorldPhysicObjectData(EntityManager* entity_manager);

	void AddPhysicObject(SceneIndex scene_index, Entity entity, const PhysicObjectBodyType& physic_object_body_type);
	void AddBoxCollider(SceneIndex scene_index, Entity entity, const Vector2& half_box_size, bool trigger = false);
	void AddCircleCollider(SceneIndex scene_index, Entity entity, float circle_radius, bool trigger = false);
	void AddBoxPhysicObject(SceneIndex scene_index, Entity entity, const PhysicObjectBodyType& physic_object_body_type, const Vector2& half_box_size, bool trigger = false);
	void AddCirclePhysicObject(SceneIndex scene_index, Entity entity, const PhysicObjectBodyType& physic_object_body_type, float circle_radius, bool trigger = false);

	void RemovePhysicObject(SceneIndex scene_index, Entity entity);
	void RemoveBoxCollider(SceneIndex scene_index, Entity entity);
	void RemoveCircleCollider(SceneIndex scene_index, Entity entity);

	void RemoveDeferredPhysicObjects(EntityManager* entity_manager);
};

