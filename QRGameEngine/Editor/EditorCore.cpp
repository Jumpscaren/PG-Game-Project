#include "pch.h"
#include "EditorCore.h"
#include "SceneSystem/SceneManager.h"
#include "Components/TransformComponent.h"
#include "Components/CameraComponent.h"
#include "Input/Mouse.h"
#include "Input/Keyboard.h"
#include "Time/Time.h"
#include "Renderer/ImGUIMain.h"
#include "IO/Output.h"
#include "Components/SpriteComponent.h"
#include "Renderer/RenderCore.h"
#include "Components/ScriptComponent.h"
#include "SceneSystem/SceneLoader.h"

EditorCore* EditorCore::s_editor_core = nullptr;

EditorCore::EditorCore()
{
	s_editor_core = this;

	EntityManager* em = SceneManager::GetSceneManager()->GetScene(SceneManager::GetSceneManager()->GetActiveSceneIndex())->GetEntityManager();

	m_editor_camera_ent = em->NewEntity();
	em->AddComponent<TransformComponent>(m_editor_camera_ent, Vector3(0.0f, 0.0f, 0.0f));
	em->AddComponent<CameraComponent>(m_editor_camera_ent);
}

EditorCore* EditorCore::Get()
{
	return s_editor_core;
}

void EditorCore::Update()
{
	EntityManager* em = SceneManager::GetSceneManager()->GetScene(SceneManager::GetSceneManager()->GetActiveSceneIndex())->GetEntityManager();

	//Editor Camera Movement Temporary Placement
	Vector3 editor_camera_pos = em->GetComponent<TransformComponent>(m_editor_camera_ent).GetPosition();

	float camera_speed = editor_camera_pos.z * (float)Time::GetDeltaTime();

	if (Keyboard::Get()->GetKeyDown(Keyboard::Key::D))
		editor_camera_pos.x += camera_speed;

	if (Keyboard::Get()->GetKeyDown(Keyboard::Key::A))
		editor_camera_pos.x -= camera_speed;

	if (Keyboard::Get()->GetKeyDown(Keyboard::Key::W))
		editor_camera_pos.y += camera_speed;

	if (Keyboard::Get()->GetKeyDown(Keyboard::Key::S))
		editor_camera_pos.y -= camera_speed;

	if (Keyboard::Get()->GetKeyDown(Keyboard::Key::R))
	{
		editor_camera_pos.x = 0;
		editor_camera_pos.y = 0;
		editor_camera_pos.z = 1;
	}

	if (Mouse::Get()->GetMouseWheelSpinDirection(Mouse::MouseWheelSpin::UP))
	{
		editor_camera_pos.z -= 1.0f;
	}
	if (Mouse::Get()->GetMouseWheelSpinDirection(Mouse::MouseWheelSpin::DOWN))
	{
		editor_camera_pos.z += 1.0f;
	}

	if (editor_camera_pos.z < 1.0f)
		editor_camera_pos.z = 1.0f;

	em->GetComponent<TransformComponent>(m_editor_camera_ent).SetPosition(editor_camera_pos);

	m_draw_scene.Update();
}

Entity EditorCore::GetEditorCameraEntity() const
{
	return m_editor_camera_ent;
}
