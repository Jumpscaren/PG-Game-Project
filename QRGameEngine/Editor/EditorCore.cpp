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

	Vector2u mouse_coords = Mouse::Get()->GetMouseCoords();
	CameraComponentInterface::ScreenToWorld(em->GetComponent<CameraComponent>(m_editor_camera_ent), Vector2((float)mouse_coords.x, (float)mouse_coords.y));

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

	bool save_pressed = false;
	bool load_pressed = false;
	bool hovering_window = false;
	ImGui::Begin("Draw Blocks");
	{
		save_pressed = ImGui::Button("Save", {0,0});
		load_pressed = ImGui::Button("Load", { 0,0 });
		hovering_window = ImGui::IsWindowHovered();
		//ImGui::Text("Average Frame Time: %f ms", average_frame_time);
		//ImGui::Text("Camera Position: x = %f, y = %f, z = %f", editor_camera_position.x, editor_camera_position.y, editor_camera_position.z);
	}
	ImGui::End();

	if (!hovering_window && !save_pressed && !load_pressed)
		m_draw_scene.Update();
	if (save_pressed)
	{
		m_draw_scene.Save();
	}
	if (load_pressed)
	{
		m_draw_scene.Load();
	}
}

Entity EditorCore::GetEditorCameraEntity() const
{
	return m_editor_camera_ent;
}
