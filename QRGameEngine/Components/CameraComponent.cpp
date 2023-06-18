#include "pch.h"
#include "CameraComponent.h"
#include "Scripting/CSMonoCore.h"
#include "SceneSystem/SceneManager.h"

void CameraComponentInterface::RegisterInterface(CSMonoCore* mono_core)
{
	auto camera_class = mono_core->RegisterMonoClass("ScriptProject.Engine", "Camera");

	mono_core->HookAndRegisterMonoMethodType<CameraComponentInterface::AddCameraComponent>(camera_class, "InitComponent", CameraComponentInterface::AddCameraComponent);
}

void CameraComponentInterface::AddCameraComponent(CSMonoObject object, SceneIndex scene_index, Entity entity)
{
	SceneManager::GetSceneManager()->GetScene(scene_index)->GetEntityManager()->AddComponent<CameraComponent>(entity);
}
