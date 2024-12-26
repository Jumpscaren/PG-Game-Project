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
#include "Components/PolygonColliderComponent.h"
#include "Input/Keyboard.h"

qr::unordered_map<std::string, std::vector<PrefabAndTextureData>> DrawScene::m_user_prefabs;
std::string DrawScene::m_category_in_use;

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
	entity_manager->AddComponent<SpriteComponent>(new_block).texture_handle = RenderCore::Get()->LoadTexture("../QRGameEngine/Textures/Temp.png", scene_manager->GetActiveSceneIndex());

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

	struct WindowGUIData
	{
		Vector2 window_position;
		float window_height, window_width;
	};

	std::vector<WindowGUIData> window_data;

	ImGui::Begin("Draw Blocks");
	{
		ImGui::InputText("Scene Name", (char*)m_scene_name.c_str(), m_scene_name.size());
		save_pressed = ImGui::Button("Save", { 0,0 });
		if (ImGui::BeginCombo("##Scenes", m_scene_name.c_str()))
		{
			auto path = std::filesystem::current_path();
			for (const auto& entry : std::filesystem::directory_iterator(path))
			{
				if (entry.path().extension() == ".scene")
				{
					const auto filename = entry.path().filename().replace_extension().string();
					const bool is_selected = (m_scene_name == filename);
					if (ImGui::Selectable(filename.c_str(), is_selected))
					{
						m_scene_name = filename;
						m_scene_name.resize(50);
					}
					if (is_selected)
					{
						ImGui::SetItemDefaultFocus();
					}
				}
			}
			ImGui::EndCombo();
		}

		load_pressed = ImGui::Button("Load", { 0,0 });
		clear_pressed = ImGui::Button("Clear");

		WindowGUIData new_window_data;
		new_window_data.window_position = Vector2(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y);
		new_window_data.window_height = ImGui::GetWindowHeight();
		new_window_data.window_width = ImGui::GetWindowWidth();
		window_data.push_back(new_window_data);

		select_pressed = ImGui::Button("Select");
		animation_pressed = ImGui::Button("Animation");
	}
	ImGui::End();

	ImGui::Begin("Prefabs");
	{
		for (const auto& category : m_user_prefabs)
		{
			ImGui::SameLine();
			if (ImGui::Button(category.first.c_str()))
			{
				m_category_in_use = category.first;
			}
		}

		const auto& it = m_user_prefabs.find(m_category_in_use);

		for (int i = 0; i < it->second.size(); ++i)
		{
			if (i % 3 == 0)
			{
				ImGui::Text(it->second[i].prefab_data.prefab_name.c_str());
				if (i + 1 < it->second.size())
				{
					ImGui::SameLine();
					ImGui::Text(it->second[i + 1].prefab_data.prefab_name.c_str());
				}
				if (i + 2 < it->second.size())
				{
					ImGui::SameLine();
					ImGui::Text(it->second[i + 2].prefab_data.prefab_name.c_str());
				}
			}

			if (it->second[i].texture_handle != -1 && ImGUIMain::ImageButton("Prefab Click" + std::to_string(i), it->second[i].texture_handle))
			{
				m_prefab_selected = it->second[i].prefab_data;
				m_select = false;
			}
			else if (it->second[i].texture_handle == -1 && ImGui::Button(it->second[i].prefab_data.prefab_name.c_str(), ImVec2(108.0f, 108.0f)))
			{
				m_prefab_selected = it->second[i].prefab_data;
				m_select = false;
			}

			if (i % 3 != 2)
			{
				ImGui::SameLine();
			}
		}

		WindowGUIData new_window_data;
		new_window_data.window_position = Vector2(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y);
		new_window_data.window_height = ImGui::GetWindowHeight();
		new_window_data.window_width = ImGui::GetWindowWidth();
		window_data.push_back(new_window_data);

		//for (int i = 0; i < m_user_prefabs.size(); ++i)
		//{
		//	if (i % 3 == 0)
		//	{
		//		ImGui::Text(m_user_prefabs[i].prefab_data.prefab_name.c_str());
		//		if (i + 1 < m_user_prefabs.size())
		//		{
		//			ImGui::SameLine();
		//			ImGui::Text(m_user_prefabs[i + 1].prefab_data.prefab_name.c_str());
		//		}
		//		if (i + 2 < m_user_prefabs.size())
		//		{
		//			ImGui::SameLine();
		//			ImGui::Text(m_user_prefabs[i + 2].prefab_data.prefab_name.c_str());
		//		}
		//	}

		//	if (m_user_prefabs[i].texture_handle != -1 && ImGUIMain::ImageButton("Prefab Click" + std::to_string(i), m_user_prefabs[i].texture_handle))
		//	{
		//		m_prefab_selected = m_user_prefabs[i].prefab_data;
		//		m_select = false;
		//	}
		//	else if (m_user_prefabs[i].texture_handle == -1 && ImGui::Button(m_user_prefabs[i].prefab_data.prefab_name.c_str(), ImVec2(108.0f, 108.0f)))
		//	{
		//		m_prefab_selected = m_user_prefabs[i].prefab_data;
		//		m_select = false;
		//	}

		//	if (i % 3 != 2)
		//	{
		//		ImGui::SameLine();
		//	}
		//}
	}
	ImGui::End();

	for (const auto& gui_window_data : window_data)
	{
		Vector2 window_x_min_max(gui_window_data.window_position.x, gui_window_data.window_position.x + gui_window_data.window_width);
		Vector2 window_y_min_max(gui_window_data.window_position.y, gui_window_data.window_position.y + gui_window_data.window_height);

		if (mouse_coords.x >= window_x_min_max.x && mouse_coords.x <= window_x_min_max.y
			&& mouse_coords.y >= window_y_min_max.x && mouse_coords.y <= window_y_min_max.y)
		{
			hovering_window = true;
		}
	}
	m_in_editor_menu = hovering_window;

	if (m_select)
	{
		Select();
		PolygonShaper();
	}
	else
	{
		ClearPolygonShaper();
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

			qr::unordered_map<uint32_t, BlockData> block_to_index_map = {};
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
	else if (json.IsObjectInteger(object_name))
	{
		int32_t temp;
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

void DrawScene::ClearPolygonShaper()
{
	SceneManager* scene_manager = SceneManager::GetSceneManager();
	EntityManager* entity_manager = scene_manager->GetEntityManager(scene_manager->GetActiveSceneIndex());

	m_in_polygon_builder = false;

	for (const auto pol_ent : m_polygon_vertices)
	{
		entity_manager->RemoveEntity(pol_ent.ent);
	}
	m_polygon_vertices.clear();
	m_polygon_vertex.ent = NULL_ENTITY;
}

void DrawScene::AddPolygonEntity(const Vector2& point, const Vector3& select_position, const uint32_t index)
{
	SceneManager* scene_manager = SceneManager::GetSceneManager();
	EntityManager* entity_manager = scene_manager->GetEntityManager(scene_manager->GetActiveSceneIndex());

	Entity ent = entity_manager->NewEntity();
	entity_manager->AddComponent<TransformComponent>(ent).SetPosition(Vector3(point.x + select_position.x, point.y + select_position.y, 0.1f)).SetScale(Vector3(0.075f, 0.075f, 1.0f));
	entity_manager->AddComponent<SpriteComponent>(ent).texture_handle = RenderCore::Get()->LoadTexture("../QRGameEngine/Textures/Temp_2.png", scene_manager->GetActiveSceneIndex());

	m_polygon_vertices.push_back(PolygonEntity{ .ent = ent, .vertex_index = index });
}

void DrawScene::Select()
{
	if (m_in_polygon_builder)
	{
		return;
	}

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

	if (!InEditorMenu() && !entity_manager->EntityExists(m_polygon_vertex.ent) && Mouse::Get()->GetMouseButtonPressed(Mouse::MouseButton::LEFT))
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
		if (ImGui::BeginCombo("##Animations", m_animation_file_name.c_str()))
		{
			auto path = std::filesystem::current_path() / std::filesystem::path("Animations");
			for (const auto& entry : std::filesystem::directory_iterator(path))
			{
				if (entry.path().extension() == ".anim")
				{
					const auto filename = entry.path().filename().replace_extension().string();
					const bool is_selected = (m_animation_file_name == filename);
					if (ImGui::Selectable(filename.c_str(), is_selected))
					{
						m_animation_file_name = filename;
					}
					if (is_selected)
					{
						ImGui::SetItemDefaultFocus();
					}
				}
			}
			ImGui::EndCombo();
		}
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
		sprite.texture_handle = RenderCore::Get()->LoadTexture(texture_full_path, SceneManager::GetSceneManager()->GetActiveSceneIndex());
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

bool show_triangles = false;
void DrawScene::PolygonShaper()
{
	Entity editor_camera = EditorCore::Get()->GetEditorCameraEntity();
	SceneManager* scene_manager = SceneManager::GetSceneManager();

	EntityManager* entity_manager = scene_manager->GetEntityManager(scene_manager->GetActiveSceneIndex());
	CameraComponent editor_camera_component = scene_manager->GetEntityManager(GlobalScene::Get()->GetSceneIndex())->GetComponent<CameraComponent>(editor_camera);

	if (entity_manager->EntityExists(m_select_entity))
	{
		if (!entity_manager->HasComponent<PolygonColliderComponent>(m_select_entity))
		{
			ClearPolygonShaper();
			m_switch_between = false;
			return;
		}

		bool add = Keyboard::Get()->GetKeyPressed(Keyboard::Key::NUM1);
		bool remove = Keyboard::Get()->GetKeyPressed(Keyboard::Key::NUM2);
		bool switch_between = Keyboard::Get()->GetKeyPressed(Keyboard::Key::NUM3);
		bool stop = Keyboard::Get()->GetKeyPressed(Keyboard::Key::NUM4);
		bool show = Keyboard::Get()->GetKeyPressed(Keyboard::Key::NUM5);
		bool reverse_order = Keyboard::Get()->GetKeyPressed(Keyboard::Key::NUM6);

		ImGui::Begin("Polygon Editor");
		{
			ImGui::Text("Add Vertex - Press 1");
			ImGui::Text("Remove Current Vertex - Press 2");
			ImGui::Text("Switch - Press 3");
			ImGui::Text("Exit - Press 4");
			ImGui::Text("Show - Press 5");
			ImGui::Text("Reverse Order - Press 6");
		}
		ImGui::End();

		if (switch_between)
		{
			m_switch_between = !m_switch_between;
			if (m_switch_between)
			{
				ClearPolygonShaper();
				return;
			}
		}

		if (stop)
		{
			ClearPolygonShaper();
			m_select_entity = NULL_ENTITY;
			m_select = false;

			return;
		}

		if (show)
		{
			show_triangles = !show_triangles;
		}

		if (m_switch_between)
		{
			return;
		}

		const TransformComponent& select_transform = entity_manager->GetComponent<TransformComponent>(m_select_entity);
		PolygonColliderComponent& polygon = entity_manager->GetComponent<PolygonColliderComponent>(m_select_entity);
		const Vector3 select_position = select_transform.GetPosition();

		if (reverse_order)
		{
			std::ranges::reverse(polygon.points);
			ClearPolygonShaper();
			return;
		}

		if (!m_in_polygon_builder)
		{
			uint32_t index = 0;
			for (Vector2& point : polygon.points)
			{
				AddPolygonEntity(point, select_position, index);
				++index;
			}
			m_in_polygon_builder = true;
		}

		bool selected_vertex = false;
		Vector3 world_mouse_position = CameraComponentInterface::ScreenToWorld(editor_camera_component, Vector2((float)Mouse::Get()->GetMouseCoords().x, (float)Mouse::Get()->GetMouseCoords().y));
		if (!entity_manager->EntityExists(m_polygon_vertex.ent))
		{
			for (const auto pol_ent : m_polygon_vertices)
			{
				const TransformComponent& transform = entity_manager->GetComponent<TransformComponent>(pol_ent.ent);
				const Vector3 pol_ent_pos = transform.GetPosition();
				if (transform.GetPosition().x - transform.GetScale().x <= world_mouse_position.x && transform.GetPosition().x + transform.GetScale().x >= world_mouse_position.x &&
					transform.GetPosition().y - transform.GetScale().y <= world_mouse_position.y && transform.GetPosition().y + transform.GetScale().y >= world_mouse_position.y &&
					Mouse::Get()->GetMouseButtonPressed(Mouse::MouseButton::LEFT))
				{
					m_polygon_vertex = pol_ent;
					selected_vertex = true;
				}
			}

			if (show_triangles)
			{
				const auto triangles = PolygonColliderComponentInterface::CreatePolygonTriangulation(m_select_entity, entity_manager);//CutTriangulation(normal);
				for (const auto& triangle : triangles)
				{
					const Vector3 p1 = Vector3(triangle.prev_point.x + select_position.x, triangle.prev_point.y + select_position.y, 0.1f);
					const Vector3 i2 = Vector3(triangle.point.x + select_position.x, triangle.point.y + select_position.y, 0.1f);
					const Vector3 n3 = Vector3(triangle.next_point.x + select_position.x, triangle.next_point.y + select_position.y, 0.1f);

					RenderCore::Get()->AddLine(Vector2(p1.x, p1.y));
					RenderCore::Get()->AddLine(Vector2(i2.x, i2.y));

					RenderCore::Get()->AddLine(Vector2(p1.x, p1.y));
					RenderCore::Get()->AddLine(Vector2(n3.x, n3.y));

					RenderCore::Get()->AddLine(Vector2(i2.x, i2.y));
					RenderCore::Get()->AddLine(Vector2(n3.x, n3.y));
				}
			}
		}

		if (add) //&& m_polygon_vertices.size() < 8)
		{
			const Vector3 prev_position = entity_manager->GetComponent<TransformComponent>(m_polygon_vertices[m_polygon_vertices.size() - 2].ent).GetPosition();
			const Vector3 back_position = entity_manager->GetComponent<TransformComponent>(m_polygon_vertices.back().ent).GetPosition();
			const Vector3 direction = (back_position - prev_position).Normalize();
			const Vector2 new_point(back_position.x + direction.x - select_position.x, back_position.y + direction.y - select_position.y);
			AddPolygonEntity(new_point, select_position, (uint32_t)m_polygon_vertices.size());
			polygon.points.push_back(new_point);
		}

		if (remove && entity_manager->EntityExists(m_polygon_vertex.ent) && m_polygon_vertices.size() > 3)
		{
			const auto index = m_polygon_vertex.vertex_index;
			for (int i = index; i < m_polygon_vertices.size(); ++i)
			{
				m_polygon_vertices[i].vertex_index = i - 1;
			}

			entity_manager->RemoveEntity(m_polygon_vertex.ent);
			m_polygon_vertices.erase(m_polygon_vertices.begin() + index);
			polygon.points.erase(polygon.points.begin() + index);

			m_polygon_vertex.ent = NULL_ENTITY;
		}

		if (entity_manager->EntityExists(m_polygon_vertex.ent))
		{
			TransformComponent& transform = entity_manager->GetComponent<TransformComponent>(m_polygon_vertex.ent);

			Vector3 position = transform.GetPosition();

			Vector2& point = polygon.points[m_polygon_vertex.vertex_index];
			Vector3 diff = world_mouse_position - position;
			transform.SetPosition(Vector3(world_mouse_position.x, world_mouse_position.y, 0.1f));

			point = point + Vector2(diff.x, diff.y);
			if (!selected_vertex && Mouse::Get()->GetMouseButtonPressed(Mouse::MouseButton::LEFT))
			{
				m_polygon_vertex.ent = NULL_ENTITY;
			}
		}

		polygon.update_polygon_collider = true;
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

void DrawScene::AddUserPrefab(const std::string& prefab_name, uint32_t z_index, const std::string& category)
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

	entity_manager->AddComponent<SpriteComponent>(ent).texture_handle = RenderCore::Get()->LoadTexture("../QRGameEngine/Textures/Temp.png", scene_manager->GetActiveSceneIndex());

	SceneLoader::Get()->InstancePrefab(game_object, prefab_name);
	
	if (entity_manager->HasComponent<SpriteComponent>(ent))
	{
		prefab_and_texture_data.texture_handle = entity_manager->GetComponent<SpriteComponent>(ent).texture_handle;
	}
	else
	{
		prefab_and_texture_data.texture_handle = -1;
	}

	entity_manager->RemoveEntity(ent);

	if (m_user_prefabs.size() == 0)
	{
		m_category_in_use = category;
	}

	auto it = m_user_prefabs.find(category);
	if (it == m_user_prefabs.end())
	{
		m_user_prefabs.insert({category, {prefab_and_texture_data}});
	}
	else
	{
		it->second.push_back(prefab_and_texture_data);
	}
}
