#pragma once
#include "Common/EngineTypes.h"
#include "ECS/EntityManager.h"
#include "Scripting/CSMonoObject.h"

class CSMonoCore;

struct TransformComponent
{
	TransformComponent(const Vector3& position = {}, const Vector3& rotation = {}, const Vector3& scale = {1.0f, 1.0f, 1.0f});

	TransformComponent& SetPosition(const Vector3& position);
	TransformComponent& SetPosition(const Vector2& position);
	TransformComponent& SetRotation(const Vector3& rotation);
	TransformComponent& SetScale(const Vector3& scale);

	Vector3 GetPosition() const;
	Vector4 GetRotation() const;
	Vector3 GetScale() const;

	DirectX::XMMATRIX world_matrix;
};

class TransformComponentInterface
{
	static MonoClassHandle vector2_class_handle;

public:
	static void RegisterInterface(CSMonoCore* mono_core);

	static void AddTransformComponent(CSMonoObject object, SceneIndex scene_index, Entity entity);

	static void SetPosition(SceneIndex scene_index, Entity entity, float x, float y);

	static CSMonoObject GetPosition(CSMonoObject cs_transform);
};

