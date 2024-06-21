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
#include "SceneSystem/GlobalScene.h"
#include "Input/Mouse.h"
#include "Components/EntityDataComponent.h"
#include "IO/JsonObject.h"
#include "SceneSystem/SceneManager.h"
#include "Components/TransformComponent.h"
#include "Components/SpriteComponent.h"
#include "Components/AnimatableSpriteComponent.h"
#include <filesystem>
#include "Animation/AnimationManager.h"

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

	SceneLoader::Get()->InstancePrefab(game_object, m_prefab_selected.prefab_name);

	BlockData new_block_data;
	new_block_data.block_entity = new_block;
	new_block_data.prefab_data = m_prefab_selected;

	return new_block_data;
}

DrawScene::DrawScene()
{
	SetAddUserPrefab();
	m_scene_name.resize(50);
	m_animation_texture_name.resize(50);
	m_animation_file_name.resize(50);
	m_select = false;
}

void DrawScene::Update()
{
	Vector2u mouse_coords = Mouse::Get()->GetMouseCoords();
	
	bool save_pressed = false;
	bool load_pressed = false;
	bool clear_pressed = false;
	bool select_pressed = false;
	bool animation_pressed = false;
	bool hovering_window = false;
	Vector2 window_position;
	float window_height, window_width;

	ImGui::Begin("Draw Blocks");
	{
		ImGui::InputText("Scene Name", (char*)m_scene_name.c_str(), m_scene_name.size());
		save_pressed = ImGui::Button("Save", { 0,0 });
		load_pressed = ImGui::Button("Load", { 0,0 });
		clear_pressed = ImGui::Button("Clear");
		window_position = Vector2(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y);
		window_height = ImGui::GetWindowHeight();
		window_width = ImGui::GetWindowWidth();

		select_pressed = ImGui::Button("Select");
		animation_pressed = ImGui::Button("Animation");

		for (int i = 0; i < m_user_prefabs.size(); ++i)
		{
			if (ImGUIMain::ImageButton("Prefab Click" + std::to_string(i), m_user_prefabs[i].texture_handle))
			{
				m_prefab_selected = m_user_prefabs[i].prefab_data;
				m_select = false;
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
	m_in_editor_menu = hovering_window;

	if (m_select)
	{
		Select();
	}
	if (m_in_animation)
	{
		Animation();
	}
	if (select_pressed)
	{
		m_select = select_pressed;
	}
	if (animation_pressed)
	{
		m_in_animation = animation_pressed;
	}
	if (m_in_animation)
		return;
	if (!hovering_window && !m_select && !m_in_animation)
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

	EntityManager* entity_manager = scene_manager->GetEntityManager(scene_manager->GetActiveSceneIndex());
	CameraComponent editor_camera_component = scene_manager->GetEntityManager(GlobalScene::Get()->GetSceneIndex())->GetComponent<CameraComponent>(editor_camera);

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

	m_blocks = SceneLoader::Get()->LoadSceneEditor(scene_name);
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

void DrawScene::WriteData(JsonObject& json, const std::string& object_name)
{
	auto it = m_names_already_in_use.find(object_name);
	std::string label_string = object_name;
	if (it == m_names_already_in_use.end())
	{
		m_names_already_in_use.insert({object_name, 0});
	}
	else
	{
		label_string += std::to_string(it->second++);
	}

	if (json.IsObjectBool(object_name))
	{
		bool temp;
		json.LoadData(temp, object_name);
		ImGui::Checkbox(label_string.c_str(), (bool*)&temp);
		json.SetData(temp, object_name);
	}
	if (json.IsObjectUnsigned(object_name))
	{
		uint32_t temp;
		json.LoadData(temp, object_name);
		ImGui::InputInt(label_string.c_str(), (int*)&temp);
		json.SetData(temp, object_name);
	}
	if (json.IsObjectFloat(object_name))
	{
		float temp;
		json.LoadData(temp, object_name);
		ImGui::InputFloat(label_string.c_str(), (float*)&temp);
		json.SetData(temp, object_name);
	}
	if (json.IsObjectString(object_name))
	{
		std::string temp;
		json.LoadData(temp, object_name);
		temp.resize(50);
		ImGui::InputText(label_string.c_str(), (char*)temp.c_str(), temp.size());
		json.SetData(temp, object_name);
	}
	if (json.IsObject(object_name))
	{
		ImGui::Text(object_name.c_str());
		JsonObject sub_json = json.GetSubJsonObject(object_name);
		std::vector<std::string> sub_object_names = sub_json.GetObjectNames();
		for (const std::string& sub_object_name : sub_object_names)
		{
			WriteData(sub_json, sub_object_name);
		}
	}
}

void DrawScene::Select()
{
	Entity editor_camera = EditorCore::Get()->GetEditorCameraEntity();
	SceneManager* scene_manager = SceneManager::GetSceneManager();

	EntityManager* entity_manager = scene_manager->GetEntityManager(scene_manager->GetActiveSceneIndex());
	CameraComponent editor_camera_component = scene_manager->GetEntityManager(GlobalScene::Get()->GetSceneIndex())->GetComponent<CameraComponent>(editor_camera);

	if (entity_manager->EntityExists(m_select_entity))
	{
		ImGui::Begin("Block Data");
		{
			std::vector<std::string> component_names = entity_manager->GetComponentNameList(m_select_entity);
			m_names_already_in_use.clear();
			for (const std::string& component_name : component_names)
			{
				ImGui::Separator();
				ImGui::Text(component_name.c_str());
				ImGui::Separator();

				const auto save_method = SceneLoader::Get()->GetOverrideSaveComponentMethod(component_name);
				if (save_method != nullptr)
				{
					JsonObject json;
					(*save_method)(m_select_entity, entity_manager, &json);
					std::vector<std::string> object_names = json.GetObjectNames();
					for (const std::string& object_name : object_names)
					{
						WriteData(json, object_name);
					}
					const auto load_method = SceneLoader::Get()->GetOverrideLoadComponentMethod(component_name);
					if (load_method != nullptr)
					{
						(*load_method)(m_select_entity, entity_manager, &json);
					}
				}
			}
			ImGui::Separator();

			const auto window_position = Vector2(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y);
			const auto window_height = ImGui::GetWindowHeight();
			const auto window_width = ImGui::GetWindowWidth();
			const Vector2 window_x_min_max(window_position.x, window_position.x + window_width);
			const Vector2 window_y_min_max(window_position.y, window_position.y + window_height);
			const Vector2u mouse_coords = Mouse::Get()->GetMouseCoords();
			if (mouse_coords.x >= window_x_min_max.x && mouse_coords.x <= window_x_min_max.y
				&& mouse_coords.y >= window_y_min_max.x && mouse_coords.y <= window_y_min_max.y)
			{
				m_in_editor_menu = true;
			}
		}
		ImGui::End();
	}

	if (!InEditorMenu() && Mouse::Get()->GetMouseButtonPressed(Mouse::MouseButton::LEFT))
	{
		const Vector3 world_mouse_position = GetWorldPositionFromMouse(editor_camera_component);

		const uint64_t unique_number = GetNumberFromPosition(world_mouse_position);

		if (m_blocks.contains(unique_number))
		{
			const auto list = m_blocks.find(unique_number)->second;
			Entity selected_entity = list.begin()->second.block_entity;
			if (selected_entity == m_select_entity && list.size() > 1)
			{
				auto it = list.begin();
				it++;
				selected_entity = it->second.block_entity;
			}
			m_select_entity = selected_entity;
		}
	}
}

void DrawScene::Animation()
{
	auto* ent_man = SceneManager::GetSceneManager()->GetScene(SceneManager::GetSceneManager()->GetActiveSceneIndex())->GetEntityManager();
	if (m_animation_base_entity == NULL_ENTITY)
	{
		Save(m_animation_temp_save_file_name);
		Clear();

		const auto base_ent = ent_man->NewEntity();
		ent_man->AddComponent<TransformComponent>(base_ent).SetPosition(Vector3(0.0f, 0.0f, 0.1f));
		ent_man->AddComponent<SpriteComponent>(base_ent);
		ent_man->AddComponent<AnimatableSpriteComponent>(base_ent);
		m_animation_base_entity = m_select_entity = base_ent;
		return;
	}

	bool back_pressed = false;
	bool save_animation_pressed = false;
	bool load_animation_pressed = false;
	SpriteComponent& sprite = ent_man->GetComponent<SpriteComponent>(m_animation_base_entity);
	Vector2 uv_1 = sprite.uv[0];
	Vector2 uv_4 = sprite.uv[3];
	ImGui::Begin("Animation");
	{
		back_pressed = ImGui::Button("Back");
		ImGui::InputText("Animation Name", (char*)m_animation_file_name.c_str(), m_animation_file_name.size());
		save_animation_pressed = ImGui::Button("Save Animation");
		load_animation_pressed = ImGui::Button("Load Animation");
		ImGui::InputText("Texture Name", (char*)m_animation_texture_name.c_str(), m_animation_texture_name.size());
		ImGui::InputFloat("uv_1.x", (float*)&uv_1.x);
		ImGui::InputFloat("uv_1.y", (float*)&uv_1.y);
		ImGui::InputFloat("uv_4.x", (float*)&uv_4.x);
		ImGui::InputFloat("uv_4.y", (float*)&uv_4.y);
	}
	ImGui::End();

	sprite.uv[0] = uv_1;
	sprite.uv[1] = Vector2(uv_4.x, uv_1.y);
	sprite.uv[2] = Vector2(uv_1.x, uv_4.y);
	sprite.uv[3] = uv_4;

	if (back_pressed)
	{
		m_in_animation = false;
		ent_man->RemoveEntity(m_animation_base_entity);
		m_animation_base_entity = NULL_ENTITY;
		Load(m_animation_temp_save_file_name);
	}

	const std::string folder_path = "../QRGameEngine/Textures/";
	const std::string texture_full_path = folder_path + m_animation_texture_name;
	bool texture_exists = false;
	if (std::filesystem::exists(texture_full_path) && std::filesystem::is_regular_file(texture_full_path))
	{
		sprite.texture_handle = RenderCore::Get()->LoadTexture(texture_full_path);
		texture_exists = true;
	}

	std::string fixed_animation_file_name = "Animations/";
	//So goddamn stupid!!!
	fixed_animation_file_name.insert(fixed_animation_file_name.length(), m_animation_file_name.c_str());
	fixed_animation_file_name += ".anim";

	if (save_animation_pressed && !texture_exists)
	{
		std::cout << "Animation Texture doesn't exists\n";
	}
	if (save_animation_pressed && texture_exists)
	{
		AnimationManager::Get()->SaveAnimation(SceneManager::GetSceneManager()->GetActiveSceneIndex(), m_animation_base_entity, fixed_animation_file_name);
	}
	if (load_animation_pressed)
	{
		if (AnimationManager::Get()->LoadAnimation(SceneManager::GetSceneManager()->GetActiveSceneIndex(), m_animation_base_entity, fixed_animation_file_name))
		{
			const auto& sprite_texture_path = AssetManager::Get()->GetAssetPath(RenderCore::Get()->GetTextureAssetHandle(sprite.texture_handle));
			m_animation_texture_name = sprite_texture_path.substr(folder_path.length(), sprite_texture_path.length() - folder_path.length());
		}
	}
}

bool DrawScene::InEditorMenu() const
{
	return m_in_editor_menu;
}

void DrawScene::SetAddUserPrefab()
{
	auto mono_core = CSMonoCore::Get();

	auto transform_class = mono_core->RegisterMonoClass("ScriptProject.Engine", "PrefabSystem");

	mono_core->HookAndRegisterMonoMethodType<DrawScene::AddUserPrefab>(transform_class, "AddUserPrefab", &DrawScene::AddUserPrefab);
}

void DrawScene::AddUserPrefab(const std::string& prefab_name, uint32_t z_index)
{
#ifndef _EDITOR
	return;
#endif // EDITOR

	PrefabData prefab_data;
	prefab_data.prefab_name = prefab_name;
	prefab_data.z_index = z_index;

	PrefabAndTextureData prefab_and_texture_data = {};
	prefab_and_texture_data.prefab_data = prefab_data;

	//Only to get the prefab sprite used for the prefab list
	CSMonoObject game_object = GameObjectInterface::CreateGameObject();
	Entity ent = GameObjectInterface::GetEntityID(game_object);

	SceneManager* scene_manager = SceneManager::GetSceneManager();
	EntityManager* entity_manager = scene_manager->GetScene(scene_manager->GetActiveSceneIndex())->GetEntityManager();

	entity_manager->AddComponent<SpriteComponent>(ent).texture_handle = RenderCore::Get()->LoadTexture("../QRGameEngine/Textures/Temp.png");

	SceneLoader::Get()->InstancePrefab(game_object, prefab_name);
	
	prefab_and_texture_data.texture_handle = entity_manager->GetComponent<SpriteComponent>(ent).texture_handle;

	entity_manager->RemoveEntity(ent);

	m_user_prefabs.push_back(prefab_and_texture_data);
}
