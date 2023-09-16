#include "pch.h"
#include "CameraComponent.h"
#include "Scripting/CSMonoCore.h"
#include "SceneSystem/SceneManager.h"
#include "Input/Mouse.h"
#include "Renderer/RenderCore.h"

void CameraComponentInterface::RegisterInterface(CSMonoCore* mono_core)
{
	auto camera_class = mono_core->RegisterMonoClass("ScriptProject.Engine", "Camera");

	mono_core->HookAndRegisterMonoMethodType<CameraComponentInterface::AddCameraComponent>(camera_class, "InitComponent", CameraComponentInterface::AddCameraComponent);
	mono_core->HookAndRegisterMonoMethodType<CameraComponentInterface::HasComponent>(camera_class, "HasComponent", CameraComponentInterface::HasComponent);
	mono_core->HookAndRegisterMonoMethodType<CameraComponentInterface::RemoveCameraComponent>(camera_class, "RemoveComponent", CameraComponentInterface::RemoveCameraComponent);
}

void CameraComponentInterface::AddCameraComponent(CSMonoObject object, SceneIndex scene_index, Entity entity)
{
	SceneManager::GetSceneManager()->GetScene(scene_index)->GetEntityManager()->AddComponent<CameraComponent>(entity);
}

bool CameraComponentInterface::HasComponent(CSMonoObject object, SceneIndex scene_index, Entity entity)
{
	return SceneManager::GetSceneManager()->GetScene(scene_index)->GetEntityManager()->HasComponent<CameraComponent>(entity);
}

void CameraComponentInterface::RemoveCameraComponent(CSMonoObject object, SceneIndex scene_index, Entity entity)
{
	SceneManager::GetSceneManager()->GetScene(scene_index)->GetEntityManager()->RemoveComponent<CameraComponent>(entity);
}

Vector3 CameraComponentInterface::ScreenToWorld(const CameraComponent& camera_component, const Vector2& position)
{
	Window* window = RenderCore::Get()->GetWindow();

	float x_ndc = ((float)position.x / window->GetWindowWidth() - 0.5f) * 2.0f;
	float y_ndc = -((float)position.y / window->GetWindowHeight() - 0.5f) * 2.0f;

	DirectX::XMMATRIX inv_proj = DirectX::XMMatrixInverse(nullptr, camera_component.proj_matrix);
	DirectX::XMMATRIX inv_view = DirectX::XMMatrixInverse(nullptr, camera_component.view_matrix);

	DirectX::XMVECTOR ndc_position = DirectX::XMVectorSet(x_ndc, y_ndc, 1.0f, 0.0f);
	DirectX::XMVECTOR world_position = DirectX::XMVector3Transform(ndc_position, inv_proj);
	world_position = DirectX::XMVector3Transform(world_position, inv_view);
	//std::cout << "x = " << world_position.m128_f32[0] << ", y = " << world_position.m128_f32[1] << "\n";

	return world_position;
}
