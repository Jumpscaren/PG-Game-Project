#include "pch.h"
#include "DrawScene.h"
#include "Components/CameraComponent.h"
#include "EditorCore.h"
#include "SceneSystem/SceneManager.h"
#include "Input/Mouse.h"
#include "Components/TransformComponent.h"
#include "Components/SpriteComponent.h"
#include "Renderer/RenderCore.h"
#include "Asset/AssetManager.h"
#include "Renderer/ImGUIMain.h"
#include "IO/Output.h"
#include <set>

uint64_t DrawScene::GetNumberFromPosition(const Vector3& position)
{
	uint64_t unique_number = 0;
	char* ptr_un = (char*)&unique_number;
	*((uint32_t*)(ptr_un)) = (uint32_t)(position.x / 0.5f);
	ptr_un += sizeof(uint32_t);
	*((uint32_t*)(ptr_un)) = (uint32_t)(position.y / 0.5f);

	return unique_number;
}

Vector3 DrawScene::GetWorldPositionFromMouse(const CameraComponent& editor_camera_component)
{
	Vector3 world_mouse_position = CameraComponentInterface::ScreenToWorld(editor_camera_component, Vector2((float)Mouse::Get()->GetMouseCoords().x, (float)Mouse::Get()->GetMouseCoords().y));

	world_mouse_position.z = 2.0f;

	world_mouse_position.x = float(int(world_mouse_position.x)) + (world_mouse_position.x < 0.0f ? -0.5f : 0.5f);
	world_mouse_position.y = float(int(world_mouse_position.y)) + (world_mouse_position.y < 0.0f ? -0.5f : 0.5f);

	return world_mouse_position;
}

DrawScene::DrawScene()
{
	m_current_texture_handle = RenderCore::Get()->LoadTexture("../QRGameEngine/Textures/Temp.png");
}

void DrawScene::Update()
{
	Vector2u mouse_coords = Mouse::Get()->GetMouseCoords();
	
	bool save_pressed = false;
	bool load_pressed = false;
	bool clear_pressed = false;
	bool hovering_window = false;
	Vector2 window_position;
	float window_height, window_width;
	bool texture_1 = false, texture_2 = false;

	ImGui::Begin("Draw Blocks");
	{
		save_pressed = ImGui::Button("Save", { 0,0 });
		load_pressed = ImGui::Button("Load", { 0,0 });
		clear_pressed = ImGui::Button("Clear");
		window_position = Vector2(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y);
		window_height = ImGui::GetWindowHeight();
		window_width = ImGui::GetWindowWidth();

		auto texture = RenderCore::Get()->LoadTexture("../QRGameEngine/Textures/Temp.png");
		texture_1 = ImGUIMain::ImageButton("Click", texture);
		texture = RenderCore::Get()->LoadTexture("../QRGameEngine/Textures/Temp_2.png");
		texture_2 = ImGUIMain::ImageButton("Click2", texture);
	}
	ImGui::End();

	Vector2 window_x_min_max(window_position.x, window_position.x + window_width);
	Vector2 window_y_min_max(window_position.y, window_position.y + window_height);

	if (mouse_coords.x >= window_x_min_max.x && mouse_coords.x <= window_x_min_max.y
		&& mouse_coords.y >= window_y_min_max.x && mouse_coords.y <= window_y_min_max.y)
	{
		hovering_window = true;
	}

	if (texture_1)
	{
		m_current_texture_handle = RenderCore::Get()->LoadTexture("../QRGameEngine/Textures/Temp.png");
	}

	if (texture_2)
	{
		m_current_texture_handle = RenderCore::Get()->LoadTexture("../QRGameEngine/Textures/Temp_2.png");
	}

	if (!hovering_window)
	{
		DrawBlock();
	}
	if (save_pressed)
	{
		Save();
	}
	if (load_pressed)
	{
		Load();
	}
	if (clear_pressed)
	{
		Clear();
	}
}

void DrawScene::DrawBlock()
{
	Entity editor_camera = EditorCore::Get()->GetEditorCameraEntity();
	SceneManager* scene_manager = SceneManager::GetSceneManager();

	EntityManager* entity_manager = scene_manager->GetScene(scene_manager->GetActiveSceneIndex())->GetEntityManager();
	CameraComponent editor_camera_component = entity_manager->GetComponent<CameraComponent>(editor_camera);

	if (Mouse::Get()->GetMouseButtonDown(Mouse::MouseButton::LEFT))
	{
		Vector3 world_mouse_position = GetWorldPositionFromMouse(editor_camera_component);

		uint64_t unique_number = GetNumberFromPosition(world_mouse_position);

		if (m_blocks.find(unique_number) == m_blocks.end())
		{
			Entity new_block = entity_manager->CreateEntity(scene_manager->GetActiveSceneIndex());
			entity_manager->AddComponent<TransformComponent>(new_block, world_mouse_position);
			entity_manager->AddComponent<SpriteComponent>(new_block).texture_handle = m_current_texture_handle;

			BlockData new_block_data;
			new_block_data.block_entity = new_block;
			m_blocks.insert({ unique_number, new_block_data });
		}
	}

	if (Mouse::Get()->GetMouseButtonDown(Mouse::MouseButton::RIGHT))
	{
		Vector3 world_mouse_position = GetWorldPositionFromMouse(editor_camera_component);

		uint64_t unique_number = GetNumberFromPosition(world_mouse_position);

		if (m_blocks.find(unique_number) != m_blocks.end())
		{
			entity_manager->RemoveEntity(m_blocks.find(unique_number)->second.block_entity);
			m_blocks.erase(unique_number);
		}
	}
}

void DrawScene::Save()
{
	//Fix saving other components aswell

	SceneManager* scene_manager = SceneManager::GetSceneManager();
	EntityManager* entity_manager = scene_manager->GetScene(scene_manager->GetActiveSceneIndex())->GetEntityManager();

	OutputFile save_file = Output::CreateCompressedOutputFile("SaveFile.sav");

	uint32_t numberofblocks = (uint32_t)m_blocks.size();
	save_file.Write(numberofblocks);

	std::unordered_map<TextureHandle, std::string> texture_paths;

	for (auto it = m_blocks.begin(); it != m_blocks.end(); it++)
	{
		TextureHandle texture_handle = entity_manager->GetComponent<SpriteComponent>(it->second.block_entity).texture_handle;

		if (texture_paths.find(texture_handle) == texture_paths.end())
		{
			std::string texture_path = AssetManager::Get()->GetAssetPath(RenderCore::Get()->GetTextureAssetHandle(texture_handle));
			texture_paths.insert({ texture_handle, texture_path });
		}
	}

	save_file.Write((uint32_t)texture_paths.size());
	for (auto it = texture_paths.begin(); it != texture_paths.end(); it++)
	{
		save_file.Write(it->first);
		save_file.Write((uint32_t)it->second.size());
		save_file.Write((void*)it->second.c_str(), (uint32_t)it->second.size());
	}

	for (auto it = m_blocks.begin(); it != m_blocks.end(); it++)
	{
		save_file.Write(it->first);
		
		save_file.Write(entity_manager->GetComponent<TransformComponent>(it->second.block_entity).world_matrix);

		save_file.Write(entity_manager->GetComponent<SpriteComponent>(it->second.block_entity).texture_handle);
	}

	save_file.Close();
}

void DrawScene::Load()
{
	SceneManager* scene_manager = SceneManager::GetSceneManager();
	EntityManager* entity_manager = scene_manager->GetScene(scene_manager->GetActiveSceneIndex())->GetEntityManager();

	Clear();

	OutputFile save_file = Output::LoadCompressedOutputFile("SaveFile.sav");

	uint32_t numberofblocks = save_file.Read<uint32_t>();
	
	uint32_t number_texture_paths = save_file.Read<uint32_t>();

	std::unordered_map<TextureHandle, std::string> texture_paths;

	for (uint32_t i = 0; i < number_texture_paths; ++i)
	{
		TextureHandle texture_handle = save_file.Read<TextureHandle>();
		uint32_t text_size = save_file.Read<uint32_t>();
		std::string text;
		text.resize(text_size);
		save_file.Read((void*)text.c_str(), text_size);

		texture_paths.insert({texture_handle, text});
	}

	for (uint32_t i = 0; i < numberofblocks; ++i)
	{
		Entity new_block = entity_manager->CreateEntity(scene_manager->GetActiveSceneIndex());
		TransformComponent& transform = entity_manager->AddComponent<TransformComponent>(new_block);
		SpriteComponent& sprite = entity_manager->AddComponent<SpriteComponent>(new_block);
	
		BlockData new_block_data;
		new_block_data.block_entity = new_block;
		uint64_t unique_number = save_file.Read<uint64_t>();
		m_blocks.insert({unique_number , new_block_data});

		transform.world_matrix = save_file.Read<DirectX::XMMATRIX>();
		sprite.texture_handle = RenderCore::Get()->LoadTexture(texture_paths.find(save_file.Read<TextureHandle>())->second);
		
	}

	save_file.Close();
}

void DrawScene::Clear()
{
	SceneManager* scene_manager = SceneManager::GetSceneManager();
	EntityManager* entity_manager = scene_manager->GetScene(scene_manager->GetActiveSceneIndex())->GetEntityManager();

	for (auto it = m_blocks.begin(); it != m_blocks.end(); it++)
	{
		entity_manager->RemoveEntity(it->second.block_entity);
	}
	m_blocks.clear();
}
