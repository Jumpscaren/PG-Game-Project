#include "pch.h"
#include "DrawScene.h"
#include "Components/CameraComponent.h"
#include "EditorCore.h"
#include "SceneSystem/SceneManager.h"
#include "Input/Mouse.h"
#include "Components/TransformComponent.h"
#include "Components/SpriteComponent.h"
#include "Asset/AssetManager.h"
#include "Renderer/ImGUIMain.h"
#include "IO/Output.h"

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
}

void DrawScene::Update()
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
			entity_manager->AddComponent<SpriteComponent>(new_block).texture_handle = AssetManager::Get()->LoadTexture("../QRGameEngine/Textures/Temp.png");
			
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
	SceneManager* scene_manager = SceneManager::GetSceneManager();
	EntityManager* entity_manager = scene_manager->GetScene(scene_manager->GetActiveSceneIndex())->GetEntityManager();

	OutputFile save_file = Output::CreateCompressedOutputFile("SaveFile.sav");
	//OutputFile save_file_not_compressed = Output::CreateOutputFile("SaveFile.txt");

	uint32_t numberofblocks = (uint32_t)m_blocks.size();
	save_file.Write(numberofblocks);
	//save_file_not_compressed.Write(numberofblocks);

	for (auto it = m_blocks.begin(); it != m_blocks.end(); it++)
	{
		//struct SaveBlockData
		//{
		//	uint64_t block_number;
		//	BlockData block_data;
		//} m_save_block_data;

		//m_save_block_data.block_data = it->second;
		//m_save_block_data.block_number = it->first;

		save_file.Write(it->first);
		//save_file_not_compressed.Write(it->first);
		
		save_file.Write(entity_manager->GetComponent<TransformComponent>(it->second.block_entity).world_matrix);
		
	}

	save_file.Close();
	//save_file_not_compressed.Close();
}

void DrawScene::Load()
{
	SceneManager* scene_manager = SceneManager::GetSceneManager();
	EntityManager* entity_manager = scene_manager->GetScene(scene_manager->GetActiveSceneIndex())->GetEntityManager();

	for (auto it = m_blocks.begin(); it != m_blocks.end(); it++)
	{
		entity_manager->RemoveEntity(it->second.block_entity);
	}
	m_blocks.clear();

	OutputFile save_file = Output::LoadCompressedOutputFile("SaveFile.sav");

	uint32_t numberofblocks = save_file.Read<uint32_t>();
	
	for (uint32_t i = 0; i < numberofblocks; ++i)
	{
		Entity new_block = entity_manager->CreateEntity(scene_manager->GetActiveSceneIndex());
		TransformComponent& transform = entity_manager->AddComponent<TransformComponent>(new_block);
		entity_manager->AddComponent<SpriteComponent>(new_block).texture_handle = AssetManager::Get()->LoadTexture("../QRGameEngine/Textures/Temp.png");
	
		BlockData new_block_data;
		new_block_data.block_entity = new_block;
		uint64_t unique_number = save_file.Read<uint64_t>();
		m_blocks.insert({unique_number , new_block_data});

		transform.world_matrix = save_file.Read<DirectX::XMMATRIX>();
	}

	save_file.Close();
	//if (m_blocks.find(unique_number) == m_blocks.end())
	//{
	//	Entity new_block = entity_manager->CreateEntity(scene_manager->GetActiveSceneIndex());
	//	entity_manager->AddComponent<TransformComponent>(new_block, world_mouse_position);
	//	entity_manager->AddComponent<SpriteComponent>(new_block).texture_handle = AssetManager::Get()->LoadTexture("../QRGameEngine/Textures/Temp.png");

	//	BlockData new_block_data;
	//	new_block_data.block_entity = new_block;
	//	m_blocks.insert({ unique_number, new_block_data });
	//}
}
