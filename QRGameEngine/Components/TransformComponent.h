#pragma once
#include "Common/EngineTypes.h"
#include "ECS/EntityManager.h"
#include "Scripting/CSMonoObject.h"

class CSMonoCore;
class JsonObject;

struct TransformComponent
{
	TransformComponent(const Vector3& position = {}, const Vector3& rotation = {}, const Vector3& scale = {1.0f, 1.0f, 1.0f});

	TransformComponent& SetPosition(const Vector3& position);
	TransformComponent& SetPosition(const Vector2& position);
	TransformComponent& SetRotation(const Vector3& rotation);
	TransformComponent& SetScale(const Vector3& scale);
	TransformComponent& SetPositionZ(float z);

	Vector3 GetPosition() const;
	Vector4 GetRotation() const;
	Vector3 GetRotationEuler() const;
	Vector3 GetScale() const;

	DirectX::XMMATRIX world_matrix;
	Vector3 m_scale, m_rotation;
};

struct PositionScaleRotation
{
	Vector3 position, scale, rotation;
};

class TransformComponentInterface
{
	static MonoClassHandle vector2_class_handle;

public:
	static void RegisterInterface(CSMonoCore* mono_core);
	static void AddTransformComponent(const CSMonoObject& object, SceneIndex scene_index, Entity entity);
	static bool HasComponent(const CSMonoObject& object, SceneIndex scene_index, Entity entity);
	static void RemoveTransformComponent(const CSMonoObject& object, SceneIndex scene_index, Entity entity);

public:
	static void SaveTransformComponent(Entity ent, EntityManager* entman, JsonObject* json_object);
	static void LoadTransformComponent(Entity ent, EntityManager* entman, JsonObject* json_object);

public:
	static void SetPosition(SceneIndex scene_index, Entity entity, float x, float y);
	static CSMonoObject GetPosition(SceneIndex scene_index, Entity entity);//const CSMonoObject& cs_transform);
	static void SetZIndex(const CSMonoObject& object, float z_index);
	static float GetZIndex(const CSMonoObject& object);

	static void SetLocalPosition(const SceneIndex scene_index, const Entity entity, const float x, const float y);
	static CSMonoObject GetLocalPosition(const CSMonoObject& cs_transform);
	static void SetLocalRotation(const SceneIndex scene_index, const Entity entity, const float angle);
	static float GetLocalRotation(const SceneIndex scene_index, const Entity entity);

	static void SetScale(const CSMonoObject& cs_transform, const CSMonoObject& scale);
	static CSMonoObject GetScale(const CSMonoObject& cs_transform);

public:
	static PositionScaleRotation GetDataFromWorldMatrix(const TransformComponent& transform);
};

