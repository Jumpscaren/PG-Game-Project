#pragma once
#include "ECS/EntityManager.h"
#include "Scripting/CSMonoObject.h"
#include "Common/EngineTypes.h"

class CSMonoCore;

struct CameraComponent
{
	DirectX::XMMATRIX view_matrix;
	DirectX::XMMATRIX proj_matrix;
};

class CameraComponentInterface
{
public:
	static void RegisterInterface(CSMonoCore* mono_core);

	static void AddCameraComponent(CSMonoObject object, SceneIndex scene_index, Entity entity);

	static Vector3 ScreenToWorld(const CameraComponent& camera_component, const Vector2& position);
};

