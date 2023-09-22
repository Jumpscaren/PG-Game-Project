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
#include "Scripting/CSMonoCore.h"
#include "Scripting/Objects/GameObjectInterface.h"
#include "SceneSystem/SceneLoader.h"

std::vector<PrefabAndTextureData> DrawScene::m_user_prefabs;

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

BlockData DrawScene::CreateBlock(const Vector3& block_transform)
{
	SceneManager* scene_manager = SceneManager::GetSceneManager();
	EntityManager* entity_manager = scene_manager->GetScene(scene_manager->GetActiveSceneIndex())->GetEntityManager();

	CSMonoObject game_object = GameObjectInterface::CreateGameObject();

	Entity new_block = GameObjectInterface::GetEntityID(game_object);

	entity_manager->GetComponent<TransformComponent>(new_block).SetPosition(block_transform);
	entity_manager->AddComponent<SpriteComponent>(new_block).texture_handle = RenderCore::Get()->LoadTexture("../QRGameEngine/Textures/Temp.png");

	SceneLoader::Get()->InstancePrefab(game_object, m_prefab_selected.prefab_index);

	BlockData new_block_data;
	new_block_data.block_entity = new_block;
	new_block_data.prefab_data = m_prefab_selected;

	return new_block_data;
}

DrawScene::DrawScene()
{
	SetAddUserPrefab();
	m_scene_name.resize(50);
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

	ImGui::Begin("Draw Blocks");
	{
		save_pressed = ImGui::Button("Save", { 0,0 });
		load_pressed = ImGui::Button("Load", { 0,0 });
		clear_pressed = ImGui::Button("Clear");
		window_position = Vector2(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y);
		window_height = ImGui::GetWindowHeight();
		window_width = ImGui::GetWindowWidth();

		ImGui::InputText("Scene Name", (char*)m_scene_name.c_str(), m_scene_name.size());

		for (int i = 0; i < m_user_prefabs.size(); ++i)
		{
			if (ImGUIMain::ImageButton("Prefab Click" + std::to_string(i), m_user_prefabs[i].texture_handle))
			{
				m_prefab_selected = m_user_prefabs[i].prefab_data;
			}
		}
	}
	ImGui::End();

	Vector2 window_x_min_max(window_position.x, window_position.x + window_width);
	Vector2 window_y_min_max(window_position.y, window_position.y + window_height);

	if (mouse_coords.x >= window_x_min_max.x && mouse_coords.x <= window_x_min_max.y
		&& mouse_coords.y >= window_y_min_max.x && mouse_coords.y <= window_y_min_max.y)
	{
		hovering_window = true;
	}

	if (!hovering_window)
	{
		DrawBlock();
	}
	if (save_pressed)
	{
		Save(m_scene_name);
	}
	if (load_pressed)
	{
		Load(m_scene_name);
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

		world_mouse_position.z = (float)m_prefab_selected.z_index;

		if (m_blocks.find(unique_number) == m_blocks.end())
		{
			BlockData new_block_data = CreateBlock(world_mouse_position);

			std::unordered_map<uint32_t, BlockData> block_to_index_map = {};
			block_to_index_map.insert({ m_prefab_selected.z_index, new_block_data });

			m_blocks.insert({ unique_number, block_to_index_map });
		}
		else
		{
			auto it = m_blocks.find(unique_number);
			if (it->second.find(m_prefab_selected.z_index) == it->second.end())
			{
				BlockData new_block_data = CreateBlock(world_mouse_position);

				it->second.insert({ m_prefab_selected.z_index, new_block_data});
			}
		}
	}

	if (Mouse::Get()->GetMouseButtonDown(Mouse::MouseButton::RIGHT))
	{
		Vector3 world_mouse_position = GetWorldPositionFromMouse(editor_camera_component);

		uint64_t unique_number = GetNumberFromPosition(world_mouse_position);

		if (m_blocks.find(unique_number) != m_blocks.end())
		{
			for (auto it = m_blocks.find(unique_number)->second.begin(); it != m_blocks.find(unique_number)->second.end(); it++)
			{
				entity_manager->RemoveEntity(it->second.block_entity);
			}
			m_blocks.erase(unique_number);
		}
	}
}

void DrawScene::Save(std::string scene_name)
{
	SceneLoader::Get()->SaveScene(m_blocks, scene_name);
}

void DrawScene::Load(std::string scene_name)
{
	Clear();

	m_blocks = SceneLoader::Get()->LoadScene(scene_name);
}

void DrawScene::Clear()
{
	SceneManager* scene_manager = SceneManager::GetSceneManager();
	EntityManager* entity_manager = scene_manager->GetScene(scene_manager->GetActiveSceneIndex())->GetEntityManager();

	for (auto it = m_blocks.begin(); it != m_blocks.end(); it++)
	{
		for (auto block_it = it->second.begin(); block_it != it->second.end(); block_it++)
		{
			entity_manager->RemoveEntity(block_it->second.block_entity);
		}
	}
	m_blocks.clear();
}

void DrawScene::SetAddUserPrefab()
{
	auto mono_core = CSMonoCore::Get();

	auto transform_class = mono_core->RegisterMonoClass("ScriptProject.Engine", "PrefabSystem");

	mono_core->HookAndRegisterMonoMethodType<DrawScene::AddUserPrefab>(transform_class, "AddUserPrefab", &DrawScene::AddUserPrefab);
}

void DrawScene::AddUserPrefab(uint32_t prefab_instance_id, uint32_t z_index)
{
#ifndef _EDITOR
	return;
#endif // EDITOR

	PrefabData prefab_data;
	prefab_data.prefab_index = prefab_instance_id;
	prefab_data.z_index = z_index;

	PrefabAndTextureData prefab_and_texture_data = {};
	prefab_and_texture_data.prefab_data = prefab_data;

	//Only to get the prefab sprite used for the prefab list
	CSMonoObject game_object = GameObjectInterface::CreateGameObject();
	Entity ent = GameObjectInterface::GetEntityID(game_object);

	SceneManager* scene_manager = SceneManager::GetSceneManager();
	EntityManager* entity_manager = scene_manager->GetScene(scene_manager->GetActiveSceneIndex())->GetEntityManager();

	entity_manager->AddComponent<SpriteComponent>(ent).texture_handle = RenderCore::Get()->LoadTexture("../QRGameEngine/Textures/Temp.png");

	SceneLoader::Get()->InstancePrefab(game_object, prefab_instance_id);
	
	prefab_and_texture_data.texture_handle = entity_manager->GetComponent<SpriteComponent>(ent).texture_handle;

	entity_manager->RemoveEntity(ent);

	m_user_prefabs.push_back(prefab_and_texture_data);
}
