#include "pch.h"
#include "DrawScene.h"
#include "Components/CameraComponent.h"
#include "EditorCore.h"
#include "SceneSystem/SceneManager.h"
#include "Input/Mouse.h"
#include "Components/TransformComponent.h"
#include "Components/SpriteComponent.h"
#include "Asset/AssetManager.h"

DrawScene::DrawScene()
{
}

void DrawScene::Update()
{
	Entity editor_camera = EditorCore::Get()->GetEditorCameraEntity();
	SceneManager* scene_manager = SceneManager::GetSceneManager();

	EntityManager* entity_manager = scene_manager->GetScene(scene_manager->GetActiveSceneIndex())->GetEntityManager();
	CameraComponent editor_camera_component = entity_manager->GetComponent<CameraComponent>(editor_camera);

	if (Mouse::Get()->GetMouseButtonDown(Mouse::MouseButton::LEFT))
	{
		Vector3 world_mouse_position = CameraComponentInterface::ScreenToWorld(editor_camera_component, Vector2(Mouse::Get()->GetMouseCoords().x, Mouse::Get()->GetMouseCoords().y));

		Entity new_block = entity_manager->CreateEntity(scene_manager->GetActiveSceneIndex());

		world_mouse_position.z = 2.0f;

		world_mouse_position.x = float(int(world_mouse_position.x)) + (world_mouse_position.x < 0.0f ? -0.5f : 0.5f);
		world_mouse_position.y = float(int(world_mouse_position.y)) + (world_mouse_position.y < 0.0f ? -0.5f : 0.5f);

		entity_manager->AddComponent<TransformComponent>(new_block, world_mouse_position);
		entity_manager->AddComponent<SpriteComponent>(new_block).texture_handle = AssetManager::Get()->LoadTexture("../QRGameEngine/Textures/Temp.png");
	}

}
