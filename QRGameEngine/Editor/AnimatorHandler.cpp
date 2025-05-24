#include "pch.h"
#include "AnimatorHandler.h"
#include "SceneSystem/SceneManager.h"
#include "ECS/EntityManager.h"
#include "Components/TransformComponent.h"
#include "Components/SpriteComponent.h"
#include "Components/AnimatableSpriteComponent.h"
#include "Input/Keyboard.h"
#include "Renderer/RenderCore.h"
#include "Asset/AssetManager.h"
#include "Renderer/ImGUIMain.h"
#include "SceneSystem/GlobalScene.h"
#include "Animation/AnimationManager.h"
#include "DrawScene.h"
#include <filesystem>
#include "SceneSystem/SceneLoader.h"
#include "IO/JsonObject.h"
#include "Time/Time.h"
#include "SceneSystem/SceneHierarchy.h"
#include "Components/ParentComponent.h"

AnimatorHandler::AnimatorHandler(DrawScene* draw_scene) : m_draw_scene(draw_scene)
{
	m_animation_texture_name.resize(50);
	m_animation_file_name.resize(50);
	m_component_name.resize(50);
	m_edit_component_name.resize(50);
	m_value_from_component_name.resize(50);
}

bool AnimatorHandler::AnimationTool()
{
	auto* ent_man = SceneManager::GetSceneManager()->GetScene(SceneManager::GetActiveSceneIndex())->GetEntityManager();
	if (m_animation_base_entity == NULL_ENTITY)
	{
		m_draw_scene->Save(m_animation_temp_save_file_name);
		m_draw_scene->Clear();

		const auto base_ent = ent_man->NewEntity();
		ent_man->AddComponent<TransformComponent>(base_ent).SetPosition(Vector3(0.0f, 0.0f, 0.1f));
		ent_man->AddComponent<SpriteComponent>(base_ent);
		ent_man->AddComponent<AnimatableSpriteComponent>(base_ent);
		m_animation_base_entity = base_ent;
		m_current_edit_entity = base_ent;
		return true;
	}
	m_draw_scene->SetSelectEntity(m_current_edit_entity);

	const AnimationUIData animationUIData = AnimationUI();
	SpriteComponent& sprite = ent_man->GetComponent<SpriteComponent>(m_current_edit_entity);

	sprite.uv[0] = animationUIData.uv_1;
	sprite.uv[1] = Vector2(animationUIData.uv_4.x, animationUIData.uv_1.y);
	sprite.uv[2] = Vector2(animationUIData.uv_1.x, animationUIData.uv_4.y);
	sprite.uv[3] = animationUIData.uv_4;

	if (animationUIData.back_pressed)
	{
		const auto& children = SceneHierarchy::Get()->GetAllChildren(ent_man->GetSceneIndex(), m_animation_base_entity);
		for (const Entity entity : children)
		{
			ent_man->RemoveEntity(entity);
		}
		ent_man->RemoveEntity(m_animation_base_entity);
		m_animation_base_entity = NULL_ENTITY;
		m_current_edit_entity = NULL_ENTITY;
		m_draw_scene->Load(m_animation_temp_save_file_name);
		return false;
	}

	const std::string folder_path = "../QRGameEngine/Textures/";
	const std::string texture_full_path = folder_path + m_animation_texture_name;
	if (animationUIData.texture_pressed)
	{
		if (std::filesystem::exists(texture_full_path) && std::filesystem::is_regular_file(texture_full_path))
		{
			sprite.texture_handle = RenderCore::Get()->LoadTexture(texture_full_path, SceneManager::GetActiveSceneIndex());
		}
		if (m_animation_texture_name.front() == '\0')
		{
			sprite.texture_handle = -1;
		}
	}

	std::string fixed_animation_file_name = "Animations/";
	//So goddamn stupid!!!
	fixed_animation_file_name.insert(fixed_animation_file_name.length(), m_animation_file_name.c_str());
	fixed_animation_file_name += ".anim";

	if (animationUIData.save_animation_pressed)
	{
		std::cout << "Animation Texture doesn't exists\n";
	}
	if (animationUIData.save_animation_pressed)
	{
		//AnimationManager::Get()->SaveAnimation(SceneManager::GetActiveSceneIndex(), m_animation_base_entity, fixed_animation_file_name);
		SaveAnimation(folder_path, fixed_animation_file_name);
		return true;
	}
	if (animationUIData.load_animation_pressed)
	{
		LoadAnimation(folder_path, fixed_animation_file_name);
		return true;
	}

	for (auto it : m_animation_value_sections)
	{
		const Entity entity = it.first.entity;
		const AnimationValueSection& animation_value_section = it.second;

		if (animation_value_section.animation_key_frames.empty())
		{
			continue;
		}

		int current_key_frame = 0;
		bool change_look_at = false;
		for (int i = 0; i < animation_value_section.animation_key_frames.size(); ++i)
		{
			const AnimationKeyFrame& key_frame = animation_value_section.animation_key_frames[i];
			if (m_timeline_time >= key_frame.timestamp)
			{
				change_look_at = true;
			}
			if (change_look_at && m_timeline_time < key_frame.timestamp)
			{
				break;
			}
			current_key_frame = i;
		}

		const AnimationKeyFrame& key_frame = animation_value_section.animation_key_frames[current_key_frame];

		if (m_timeline_time < key_frame.timestamp)
		{
			continue;
		}

		AnimationManager::SetAnimationKeyData(current_key_frame, animation_value_section, m_animation_value_storage, ent_man->GetSceneIndex(), entity, m_timeline_time);
	}

	return true;
}

ImGuiWindowFlags_ flag = ImGuiWindowFlags_::ImGuiWindowFlags_None;

AnimatorHandler::AnimationUIData AnimatorHandler::AnimationUI()
{
	if (m_timeline_play)
	{
		m_timeline_time += Time::GetDeltaTime();
	}
	if (m_timeline_time > m_animation_max_time && m_timeline_loop)
	{
		m_timeline_time = 0.0f;
	}

	auto* ent_man = SceneManager::GetSceneManager()->GetScene(SceneManager::GetActiveSceneIndex())->GetEntityManager();

	bool back_pressed = false;
	bool save_animation_pressed = false;
	bool load_animation_pressed = false;
	bool texture_pressed = false;
	const SpriteComponent& sprite = ent_man->GetComponent<SpriteComponent>(m_current_edit_entity);
	Vector2 uv_1 = sprite.uv[0];
	Vector2 uv_4 = sprite.uv[3];
	ImGui::Begin("Animation", 0, flag);
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
						m_animation_file_name.resize(50);
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
		m_animation_texture_name.resize(50);
		ImGui::SameLine();
		texture_pressed = ImGui::Button("Load Texture");
		ImGui::InputFloat("uv_1.x", (float*)&uv_1.x);
		ImGui::InputFloat("uv_1.y", (float*)&uv_1.y);
		ImGui::InputFloat("uv_4.x", (float*)&uv_4.x);
		ImGui::InputFloat("uv_4.y", (float*)&uv_4.y);

		ImGui::InputFloat2("Split Size", (float*)&m_split_size);
		ImGui::InputInt("Splits", &m_max_split_index);
		ImGui::InputFloat("Time Between Splits", &m_time_between_splits);

		if (ImGui::Button("Generate UV"))
		{
			for (int uv_index = 1; uv_index <= 4; ++uv_index)
			{
				AnimationValueSection* uv_section = nullptr;
				const AnimationSetterId uv_setter_id = AnimationManager::Get()->GetAnimationValueSetterStorageIndex("SpriteComponent", "UV_" + std::to_string(uv_index));
				const AnimationValueSetterStorage& uv_setter_storage = AnimationManager::Get()->GetAnimationValueSetterStorage(uv_setter_id);

				const AnimationValueSectionId uv_section_id{.setter_id = uv_setter_id, .entity = m_current_edit_entity};

				if (!m_animation_value_sections.contains(uv_section_id))
				{
					const AnimationValueSection new_section{ .value_setter_storage_id = uv_setter_id, .value_type = uv_setter_storage.value_type, .animation_key_frames = {} };
					m_animation_value_sections.insert({ uv_section_id, new_section });
				}
				uv_section = &(m_animation_value_sections.at(uv_section_id));

				Vector2 uv_position;
				if (uv_index == 1)
				{
					uv_position = uv_1;
				}
				else if (uv_index == 2)
				{
					uv_position = Vector2(uv_4.x, uv_1.y);
				}
				else if (uv_index == 3)
				{
					uv_position = Vector2(uv_1.x, uv_4.y);
				}
				else
				{
					uv_position = uv_4;
				}

				for (int i = 0; i < m_max_split_index; ++i)
				{
					const AnimationValueDataId uv_value_data_id = m_animation_value_storage.animation_value_vector2_storage.size();

					const Vector2 uv_value = uv_position + m_split_size * i;

					m_animation_value_storage.animation_value_vector2_storage.push_back(uv_value);
					uv_section->animation_key_frames.push_back(AnimationKeyFrame{ .timestamp = i * m_time_between_splits, .value_interpolation = AnimationValueInterpolation::Step, .value_data_id = uv_value_data_id });
				}
			}
		}

		if (ImGui::Button("Go to Parent") && ent_man->HasComponent<ParentComponent>(m_current_edit_entity))
		{
			m_current_edit_entity = ent_man->GetComponent<ParentComponent>(m_current_edit_entity).parent;
			m_current_child_index = 0;
		}
		const std::string entity_index = "Entity " + std::to_string(m_current_edit_entity);
		ImGui::Text(entity_index.c_str());
		if (SceneHierarchy::Get()->ParentHasChildren(ent_man->GetSceneIndex(), m_current_edit_entity))
		{
			const std::string child_index = "Child Index " + std::to_string(m_current_child_index);
			ImGui::Text(child_index.c_str());

			const std::vector<SceneHierarchy::Child> ordered_children = SceneHierarchy::Get()->GetOrderedChildren(ent_man->GetSceneIndex(), m_current_edit_entity);
			if (ImGui::Button("Previous Child"))
			{
				if (m_current_child_index != 0)
				{
					--m_current_child_index;
				}
			}
			ImGui::SameLine();
			if (ImGui::Button("Next Child"))
			{
				++m_current_child_index;
				if (m_current_child_index >= ordered_children.size())
				{
					m_current_child_index = ordered_children.size() - 1;
				}
			}
			ImGui::SameLine();
			if (ImGui::Button("Select Child"))
			{
				m_current_edit_entity = ordered_children.at(m_current_child_index).child;
				//m_animation_texture_name = AssetManager::Get()->GetAssetPath(RenderCore::Get()->GetTextureAssetHandle(ent_man->GetComponent<SpriteComponent>(m_current_edit_entity).texture_handle));
				m_current_child_index = 0;
			}
		}

		if (ImGui::Button("Add Child"))
		{
			const Entity child = ent_man->NewEntity();
			ent_man->AddComponent<TransformComponent>(child);
			ent_man->AddComponent<SpriteComponent>(child);
			SceneHierarchy::Get()->AddParentChildRelation(m_current_edit_entity, child);
		}


		ImGui::Text("Timeline");
		ImGui::InputFloat("Timeline Time", &m_timeline_time);
		ImGui::Checkbox("Loop", &m_timeline_loop);
		ImGui::SameLine();
		ImGui::Checkbox("Play", &m_timeline_play);
		ImGui::NewLine();

		ImGui::SliderFloat("Max Animation Time", &m_animation_max_time, 0.0f, 100.0f);
		ImGui::SliderFloat("Zoom", &m_animation_timeline_zoom, 0.0001f, 2.0f);
		if (ImGui::Button("Clear Animation"))
		{
			ClearAnimationData();
		}

		ImGui::InputText("Component Name", (char*)m_component_name.c_str(), m_component_name.size());
		ImGui::SameLine();
		std::string fixed_component_name = m_component_name;
		fixed_component_name.erase(std::remove(fixed_component_name.begin(), fixed_component_name.end(), 0), fixed_component_name.end());
		if (ImGui::Button("Add Component"))
		{
			if (ent_man->ComponentExists(fixed_component_name) && !ent_man->HasComponentName(m_current_edit_entity, fixed_component_name))
			{
				ent_man->AddComponent(fixed_component_name, m_current_edit_entity);
			}
		}

		if (ImGui::BeginCombo("##Edit Component", m_edit_component_name.c_str()))
		{
			for (const auto& component_name : ent_man->GetComponentNameList(m_current_edit_entity))
			{
				const bool is_selected = (m_edit_component_name == component_name);
				if (ImGui::Selectable(component_name.c_str(), is_selected))
				{
					m_edit_component_name = component_name;
				}
				if (is_selected)
				{
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}

		EditValueFromComponent();
		ManageKeyFrames();

		const auto draw_list = ImGui::GetWindowDrawList();
		auto cursor_pos = ImGui::GetCursorScreenPos();
		const auto window_pos = ImGui::GetWindowPos();
		auto canvas_size = ImGui::GetContentRegionAvail();

		draw_list->AddRectFilled(cursor_pos, ImVec2(cursor_pos.x + canvas_size.x, cursor_pos.y + canvas_size.y), ImColor(120, 120, 255, 120));

		constexpr float GRID_SIZE = 50.0f;
		float ZOOMED_GRID_SIZE = GRID_SIZE * m_animation_timeline_zoom;
		constexpr float HALF_GRID_SIZE = 25.0f;
		float ZOOMED_HALF_GRID_SIZE = HALF_GRID_SIZE * m_animation_timeline_zoom;
		constexpr float VALUE_OFFSET_X = 5.0f;
		constexpr float VALUE_OFFSET_Y = 20.0f;
		constexpr float BLACK_BOX_SIZE_X = GRID_SIZE * 2.0f;

		for (float x = 0.0f; x < canvas_size.x; x += GRID_SIZE)
		{
			draw_list->AddLine(ImVec2(cursor_pos.x + x, cursor_pos.y), ImVec2(cursor_pos.x + x, cursor_pos.y + canvas_size.y), ImColor(255, 255, 255, 255));
		}

		for (float y = 0.0f; y < canvas_size.y; y += GRID_SIZE)
		{
			draw_list->AddLine(ImVec2(cursor_pos.x, cursor_pos.y + y), ImVec2(cursor_pos.x + canvas_size.x, cursor_pos.y + y), ImColor(255, 255, 255, 255));
		}

		draw_list->AddRectFilled(cursor_pos, ImVec2(cursor_pos.x + BLACK_BOX_SIZE_X, cursor_pos.y + canvas_size.y), ImColor(25, 25, 25, 255));

		const float timeline_time_x = BLACK_BOX_SIZE_X + m_timeline_time / m_animation_timeline_zoom;
		const float max_time_x = BLACK_BOX_SIZE_X + m_animation_max_time / m_animation_timeline_zoom;
		draw_list->AddLine(ImVec2(cursor_pos.x + timeline_time_x, cursor_pos.y), ImVec2(cursor_pos.x + timeline_time_x, cursor_pos.y + canvas_size.y), ImColor(255, 0, 0, 255));

		draw_list->AddLine(ImVec2(cursor_pos.x + max_time_x, cursor_pos.y), ImVec2(cursor_pos.x + max_time_x, cursor_pos.y + canvas_size.y), ImColor(0, 0, 255, 255), 2.0f);

		int row = 0;
		for (const auto it : m_values_used_in_animation)
		{
			for (int i = 0; i < it.second.size(); ++i && ++row)
			{
				const auto& value = it.second[i];

				const std::string value_text = value.component_name.substr(0, 4) + ":" + value.value_name;
				draw_list->AddText(ImVec2(cursor_pos.x + VALUE_OFFSET_X, cursor_pos.y + row * GRID_SIZE + VALUE_OFFSET_Y), ImColor(255, 255, 255, 255), value_text.c_str());

				const AnimationValueSectionId animation_value_section_id{.setter_id = value.setter_id, .entity = it.first};
				if (!m_animation_value_sections.contains(animation_value_section_id))
				{
					continue;
				}

				const float grid_position_y = HALF_GRID_SIZE + row * GRID_SIZE;
				AnimationValueSection animation_value_section;
				AnimationValueSectionId selected_animation_value_section_id;
				for (const auto find_section : m_animation_value_sections)
				{
					const AnimationValueSection& value_section = find_section.second;
					if (find_section.first.entity == it.first && value_section.value_setter_storage_id == value.setter_id)
					{
						animation_value_section = value_section;
						selected_animation_value_section_id = AnimationValueSectionId{ .setter_id = value_section.value_setter_storage_id, .entity = find_section.first.entity };
						break;
					}
				}

				const AnimationValueSectionId current_animation_value_section_id{.setter_id = m_current_animation_setter_id, .entity = m_current_edit_entity};
				for (int key_frame_idx = 0; key_frame_idx < animation_value_section.animation_key_frames.size(); ++key_frame_idx)
				{
					const AnimationKeyFrame& key_frame = animation_value_section.animation_key_frames[key_frame_idx];

					ImColor key_frame_colour(0, 255, 0, 255);
					if (current_animation_value_section_id == selected_animation_value_section_id && m_key_frame_index == key_frame_idx)
					{
						key_frame_colour = ImColor(128, 0, 128, 255);
					}

					draw_list->AddCircleFilled(ImVec2(cursor_pos.x + BLACK_BOX_SIZE_X + key_frame.timestamp / m_animation_timeline_zoom, cursor_pos.y + grid_position_y), 5.0f, key_frame_colour);

					if (key_frame_idx + 1 >= animation_value_section.animation_key_frames.size())
					{
						continue;
					}

					const AnimationKeyFrame& next_key_frame = animation_value_section.animation_key_frames[key_frame_idx + 1];
					if (next_key_frame.value_interpolation == AnimationValueInterpolation::Linear)
					{
						draw_list->AddLine(ImVec2(cursor_pos.x + BLACK_BOX_SIZE_X + key_frame.timestamp / m_animation_timeline_zoom, cursor_pos.y + grid_position_y),
							ImVec2(cursor_pos.x + BLACK_BOX_SIZE_X + next_key_frame.timestamp / m_animation_timeline_zoom, cursor_pos.y + grid_position_y),
							ImColor(0, 255, 0, 255));
					}
				}
			}
		}

		if (ImGui::IsMouseHoveringRect(cursor_pos, ImVec2(cursor_pos.x + canvas_size.x, cursor_pos.y + canvas_size.y)))
		{
			flag = ImGuiWindowFlags_::ImGuiWindowFlags_NoMove;
		}
		else
		{
			flag = ImGuiWindowFlags_::ImGuiWindowFlags_None;
		}
	}
	ImGui::End();

	return AnimationUIData{.back_pressed = back_pressed, .save_animation_pressed = save_animation_pressed, .load_animation_pressed = load_animation_pressed, .uv_1 = uv_1, .uv_4 = uv_4, .texture_pressed = texture_pressed};
}

void AnimatorHandler::EditValueFromComponent()
{
	auto* ent_man = SceneManager::GetSceneManager()->GetScene(SceneManager::GetActiveSceneIndex())->GetEntityManager();

	std::string fixed_edit_component_name = m_edit_component_name;
	fixed_edit_component_name.erase(std::remove(fixed_edit_component_name.begin(), fixed_edit_component_name.end(), 0), fixed_edit_component_name.end());

	if (ent_man->ComponentExists(fixed_edit_component_name) && ent_man->HasComponentName(m_current_edit_entity, fixed_edit_component_name) && ImGui::BeginCombo("##Value From Component", m_value_from_component_name.c_str()))
	{
		for (const auto& animation_value_setter_storage : AnimationManager::Get()->GetAnimationValueSetterStorages())
		{
			if (animation_value_setter_storage.component_name == fixed_edit_component_name)
			{
				const bool is_selected = (m_value_from_component_name == animation_value_setter_storage.value_name);
				if (ImGui::Selectable(animation_value_setter_storage.value_name.c_str(), is_selected))
				{
					m_value_from_component_name = animation_value_setter_storage.value_name;
				}
				if (is_selected)
				{
					ImGui::SetItemDefaultFocus();
				}
			}
		}
		ImGui::EndCombo();
	}
	const AnimationSetterId value_setter_index = AnimationManager::Get()->GetAnimationValueSetterStorageIndex(m_edit_component_name, m_value_from_component_name);
	if (value_setter_index == -1)
	{
		return;
	}

	ImGui::SameLine();
	if (ImGui::Button("Add Value"))
	{
		std::string fixed_value_from_component_name = m_value_from_component_name;
		fixed_value_from_component_name.erase(std::remove(fixed_value_from_component_name.begin(), fixed_value_from_component_name.end(), 0), fixed_value_from_component_name.end());

		bool value_already_used = false;
		if (m_values_used_in_animation.contains(m_current_edit_entity))
		{
			for (const ValueInAnimation& value : m_values_used_in_animation.at(m_current_edit_entity))
			{
				if (value.setter_id == value_setter_index)
				{
					value_already_used = true;
					break;
				}
			}
		}

		if (!value_already_used)
		{
			const ValueInAnimation value_in_animation{ .component_name = fixed_edit_component_name, .value_name = fixed_value_from_component_name, .setter_id = value_setter_index };
			if (!m_values_used_in_animation.contains(m_current_edit_entity))
			{
				m_values_used_in_animation.insert({ m_current_edit_entity, {value_in_animation} });
			}
			else
			{
				m_values_used_in_animation.at(m_current_edit_entity).push_back(value_in_animation);
			}
		}
	}

	if (!m_values_used_in_animation.contains(m_current_edit_entity))
	{
		return;
	}

	const std::vector<ValueInAnimation>& values_used_in_animation = m_values_used_in_animation.at(m_current_edit_entity);

	std::string value_name = "";
	for (const ValueInAnimation& value : values_used_in_animation)
	{
		if (m_current_animation_setter_id == value.setter_id)
		{
			value_name = value.value_name;
			break;
		}
	}

	if (ImGui::BeginCombo("##Added Values", value_name.c_str()))
	{
		for (const ValueInAnimation& value : values_used_in_animation)
		{
			const bool is_selected = (m_current_animation_setter_id == value.setter_id);
			if (ImGui::Selectable(value.value_name.c_str(), is_selected))
			{
				m_current_animation_setter_id = value.setter_id;
			}
			if (is_selected)
			{
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}
}

void AnimatorHandler::ManageKeyFrames()
{
	ImGui::InputFloat("Time", &m_key_frame_time);

	const auto& value = AnimationManager::Get()->GetAnimationValueSetterStorage(m_current_animation_setter_id);
	switch (value.value_type)
	{
	case AnimationValueType::Float:
		ImGui::InputFloat("Value", &m_value_float);
		break;
	case AnimationValueType::Bool:
		ImGui::Checkbox("Value", &m_value_bool);
		break;
	case AnimationValueType::Int:
		ImGui::InputInt("Value", &m_value_int);
		break;
	case AnimationValueType::Vector2:
		ImGui::InputFloat("Value X", &m_value_vector2.x);
		ImGui::InputFloat("Value Y", &m_value_vector2.y);
		break;
	}

	const auto get_value_interpolation_name = [](const AnimationValueInterpolation interpolation) -> std::string {
		if (interpolation == AnimationValueInterpolation::Linear)
		{
			return "Linear";
		}
		else
		{
			return "Step";
		}
	};

	std::string current_value_interpolation_name = get_value_interpolation_name(m_value_interpolation);

	if (ImGui::BeginCombo("##Key Frame Lerp", current_value_interpolation_name.c_str()))
	{
		for (const AnimationValueInterpolation& interpolation : { AnimationValueInterpolation::Linear, AnimationValueInterpolation::Step })
		{
			const bool is_selected = (m_value_interpolation == interpolation);

			if (ImGui::Selectable(get_value_interpolation_name(interpolation).c_str(), is_selected))
			{
				m_value_interpolation = interpolation;
			}
			if (is_selected)
			{
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}

	const AnimationValueSectionId animation_value_section_id{ .setter_id = m_current_animation_setter_id, .entity = m_current_edit_entity };
	if (ImGui::Button("Add Key Frame"))
	{
		AnimationValueDataId value_data_id{ 0 };
		if (value.value_type == AnimationValueType::Float)
		{
			value_data_id = m_animation_value_storage.animation_value_float_storage.size();
			m_animation_value_storage.animation_value_float_storage.push_back(m_value_float);
		}
		else if (value.value_type == AnimationValueType::Bool)
		{
			value_data_id = m_animation_value_storage.animation_value_bool_storage.size();
			m_animation_value_storage.animation_value_bool_storage.push_back(m_value_bool);
		}
		else if (value.value_type == AnimationValueType::Int)
		{
			value_data_id = m_animation_value_storage.animation_value_int_storage.size();
			m_animation_value_storage.animation_value_int_storage.push_back(m_value_int);
		}
		else if (value.value_type == AnimationValueType::Vector2)
		{
			value_data_id = m_animation_value_storage.animation_value_vector2_storage.size();
			m_animation_value_storage.animation_value_vector2_storage.push_back(m_value_vector2);
		}

		AnimationValueSection* section;
		if (!m_animation_value_sections.contains(animation_value_section_id))
		{
			const AnimationValueSection new_section{ .value_setter_storage_id = m_current_animation_setter_id, .value_type = value.value_type, .animation_key_frames = {} };
			m_animation_value_sections.insert({ animation_value_section_id, new_section });
		}
		section = &(m_animation_value_sections.at(animation_value_section_id));

		const auto it = std::ranges::find_if(section->animation_key_frames, [this](const AnimationKeyFrame& key_frame) { return key_frame.timestamp > m_key_frame_time; });

		AnimationKeyFrame key_frame{ .timestamp = m_key_frame_time, .value_interpolation = m_value_interpolation , .value_data_id = value_data_id };
		if (it == section->animation_key_frames.end())
		{
			section->animation_key_frames.push_back(key_frame);
		}
		else
		{
			section->animation_key_frames.insert(it, key_frame);
		}
	}

	if (!m_animation_value_sections.contains(animation_value_section_id))
	{
		return;
	}
	AnimationValueSection& section = m_animation_value_sections.at(animation_value_section_id);
	
	if (section.animation_key_frames.empty())
	{
		return;
	}

	ImGui::SameLine();
	if (ImGui::Button("Edit Key Frame"))
	{
		AnimationKeyFrame& key_frame = section.animation_key_frames[m_key_frame_index];

		if (value.value_type == AnimationValueType::Float)
		{
			m_animation_value_storage.animation_value_float_storage[key_frame.value_data_id] = m_value_float;
		}
		else if (value.value_type == AnimationValueType::Bool)
		{
			m_animation_value_storage.animation_value_bool_storage[key_frame.value_data_id] = m_value_bool;
		}
		else if (value.value_type == AnimationValueType::Int)
		{
			m_animation_value_storage.animation_value_int_storage[key_frame.value_data_id] = m_value_int;
		}
		else if (value.value_type == AnimationValueType::Vector2)
		{
			m_animation_value_storage.animation_value_vector2_storage[key_frame.value_data_id] = m_value_vector2;
		}

		const AnimationValueDataId old_key_frame_value_data_id = key_frame.value_data_id;
		key_frame = AnimationKeyFrame{ .timestamp = m_key_frame_time, .value_interpolation = AnimationValueInterpolation::Linear , .value_data_id = old_key_frame_value_data_id };

		std::ranges::sort(section.animation_key_frames, [](const AnimationKeyFrame& a, const AnimationKeyFrame& b) { return a.timestamp < b.timestamp; });

		const auto it = std::ranges::find_if(section.animation_key_frames, [old_key_frame_value_data_id](const AnimationKeyFrame& key_frame) { return key_frame.value_data_id == old_key_frame_value_data_id; });
		m_key_frame_index = std::distance(section.animation_key_frames.begin(), it);

		key_frame.value_interpolation = m_value_interpolation;
	}

	ImGui::SameLine();
	if (ImGui::Button("Load Key Frame"))
	{
		AnimationKeyFrame& key_frame = section.animation_key_frames[m_key_frame_index];

		m_key_frame_time = key_frame.timestamp;
		if (value.value_type == AnimationValueType::Float)
		{
			m_value_float = m_animation_value_storage.animation_value_float_storage[key_frame.value_data_id];
		}
		else if (value.value_type == AnimationValueType::Bool)
		{
			m_value_bool = m_animation_value_storage.animation_value_bool_storage[key_frame.value_data_id];
		}
		else if (value.value_type == AnimationValueType::Int)
		{
			m_value_int = m_animation_value_storage.animation_value_int_storage[key_frame.value_data_id];
		}
		else if (value.value_type == AnimationValueType::Vector2)
		{
			m_value_vector2 = m_animation_value_storage.animation_value_vector2_storage[key_frame.value_data_id];
		}

		m_value_interpolation = key_frame.value_interpolation;
	}

	ImGui::SameLine();
	if (ImGui::Button("Delete Key Frame"))
	{
		section.animation_key_frames.erase(section.animation_key_frames.begin() + m_key_frame_index);
		if (--m_key_frame_index < 0)
		{
			m_key_frame_index = 0;
		}
	}

	if (ImGui::Button("Next Key Frame"))
	{
		m_key_frame_index++;
		if (m_key_frame_index >= section.animation_key_frames.size())
		{
			m_key_frame_index = section.animation_key_frames.size() - 1;
		}
	}
	ImGui::SameLine();
	if (ImGui::Button("Previous Key Frame"))
	{
		m_key_frame_index--;
		if (m_key_frame_index < 0)
		{
			m_key_frame_index = 0;
		}
	}
}

void SaveSprite(JsonObject& entity_data, Entity entity, EntityManager* entity_manager)
{
	JsonObject sprite_data = entity_data.CreateSubJsonObject("SpriteData");
	const SpriteComponent& sprite = entity_manager->GetComponent<SpriteComponent>(entity);

	std::string texture_path = "";

	if (sprite.texture_handle != -1)
	{
		texture_path = AssetManager::Get()->GetAssetPath(RenderCore::Get()->GetTextureAssetHandle(sprite.texture_handle));
		texture_path.erase(std::remove(texture_path.begin(), texture_path.end(), 0), texture_path.end());
	}

	SpriteComponentInterface::SaveSpriteComponent(entity, entity_manager, &sprite_data);
	sprite_data.SetData(texture_path, "Texture_Path");
}

void AnimatorHandler::SaveAnimatableSprite(JsonObject& entity_data, Entity entity, EntityManager* entity_manager)
{
	JsonObject animation_data = entity_data.CreateSubJsonObject("Animatable");

	for (auto it : m_animation_value_sections)
	{
		const AnimationValueSectionId& animation_value_section_id = it.first;
		const AnimationValueSection& animation_value_section = it.second;

		if (animation_value_section.animation_key_frames.empty())
		{
			continue;
		}

		if (it.first.entity != entity)
		{
			continue;
		}

		const auto value_in_animation_it = std::ranges::find_if(m_values_used_in_animation.at(animation_value_section_id.entity), [animation_value_section](const ValueInAnimation& value_in_animation)
			{
				return value_in_animation.setter_id == animation_value_section.value_setter_storage_id;
			});


		const std::string section_name = value_in_animation_it->component_name + ":" + value_in_animation_it->value_name;

		JsonObject animation_value_section_data = animation_data.CreateSubJsonObject(section_name + "-AnimationValueSection");

		animation_value_section_data.SetData((int)animation_value_section.value_type, "ValueType");

		for (int i = 0; i < animation_value_section.animation_key_frames.size(); ++i)
		{
			const AnimationKeyFrame& key_frame = animation_value_section.animation_key_frames[i];

			JsonObject key_frame_data = animation_value_section_data.CreateSubJsonObject("KeyFrame " + std::to_string(i));
			key_frame_data.SetData(key_frame.timestamp, "Timestamp");
			key_frame_data.SetData((int)key_frame.value_interpolation, "Value_Interpolation");

			if (animation_value_section.value_type == AnimationValueType::Float)
			{
				key_frame_data.SetData(m_animation_value_storage.animation_value_float_storage[key_frame.value_data_id], "Value_Float");
			}
			else if (animation_value_section.value_type == AnimationValueType::Bool)
			{
				key_frame_data.SetData(m_animation_value_storage.animation_value_bool_storage[key_frame.value_data_id], "Value_Bool");
			}
			else if (animation_value_section.value_type == AnimationValueType::Int)
			{
				key_frame_data.SetData(m_animation_value_storage.animation_value_int_storage[key_frame.value_data_id], "Value_Int");
			}
			else if (animation_value_section.value_type == AnimationValueType::Vector2)
			{
				key_frame_data.SetData(m_animation_value_storage.animation_value_vector2_storage[key_frame.value_data_id], "Value_Vector2");
			}
		}
	}
}

void AnimatorHandler::SaveChildData(JsonObject& parent_data, Entity parent, EntityManager* entity_manager)
{
	JsonObject new_parent_data = parent_data.CreateSubJsonObject("Parent");

	SaveSprite(new_parent_data, parent, entity_manager);
	SaveAnimatableSprite(new_parent_data, parent, entity_manager);

	if (!SceneHierarchy::Get()->ParentHasChildren(entity_manager->GetSceneIndex(), parent))
	{
		new_parent_data.CreateSubJsonObject("NoChildren");
		return;
	}

	const std::vector<SceneHierarchy::Child>& ordered_children = SceneHierarchy::Get()->GetOrderedChildren(entity_manager->GetSceneIndex(), parent);

	new_parent_data.SetData((uint32_t)ordered_children.size(), "ChildrenCount");
	for (int i = 0; i < ordered_children.size(); ++i)
	{
		const SceneHierarchy::Child& child = ordered_children[i];
		new_parent_data.SetData(child.child, "Child" + std::to_string(i));
		SaveChildData(new_parent_data, child.child, entity_manager);
	}
}

void AnimatorHandler::SaveAnimation(const std::string& folder_path, const std::string& animation_file_name)
{
	JsonObject save_animation;
	save_animation.CreateSubJsonObject("AnimationSystemVersion").SetData(2, "Version");

	{
		JsonObject animation_helper_data = save_animation.CreateSubJsonObject("Animation_Helper_Data");
		animation_helper_data.SetData(m_split_size, "SplitSize");
		animation_helper_data.SetData(m_max_split_index, "Splits");
		animation_helper_data.SetData(m_time_between_splits, "TimeBetweenSplits");
	}

	EntityManager* entity_manager = SceneManager::GetSceneManager()->GetEntityManager(SceneManager::GetActiveSceneIndex());

	JsonObject hierarchy_data = save_animation.CreateSubJsonObject("HierarchyData");
	hierarchy_data.SetData(m_animation_base_entity, "ParentEntity");
	SaveChildData(hierarchy_data, m_animation_base_entity, entity_manager);

	//{
	//	JsonObject sprite_data = save_animation.CreateSubJsonObject("SpriteData");
	//	const SpriteComponent& sprite = entity_manager->GetComponent<SpriteComponent>(m_animation_base_entity);

	//	std::string texture_path = "";

	//	if (sprite.texture_handle != -1)
	//	{
	//		texture_path = AssetManager::Get()->GetAssetPath(RenderCore::Get()->GetTextureAssetHandle(sprite.texture_handle));
	//		texture_path.erase(std::remove(texture_path.begin(), texture_path.end(), 0), texture_path.end());
	//	}

	//	SpriteComponentInterface::SaveSpriteComponent(m_animation_base_entity, entity_manager, &sprite_data);
	//	sprite_data.SetData(texture_path, "Texture_Path");
	//}

	{
		JsonObject animatable_sprite_data = save_animation.CreateSubJsonObject("AnimatableSpriteData");
		AnimatableSpriteComponentInterface::SaveAnimatableSpriteComponent(m_animation_base_entity, entity_manager, &animatable_sprite_data);
		animatable_sprite_data.SetData(m_animation_max_time, "AnimationTime");

		//JsonObject animation_data = save_animation.CreateSubJsonObject("Animatable");

		//for (auto it : m_animation_value_sections)
		//{
		//	const AnimationValueSectionId& animation_value_section_id = it.first;
		//	const AnimationValueSection& animation_value_section = it.second;

		//	if (animation_value_section.animation_key_frames.empty())
		//	{
		//		continue;
		//	}

		//	const auto value_in_animation_it = std::ranges::find_if(m_values_used_in_animation.at(animation_value_section_id.entity), [animation_value_section](const ValueInAnimation& value_in_animation)
		//		{ 
		//			return value_in_animation.setter_id == animation_value_section.value_setter_storage_id; 
		//		});


		//	const std::string section_name = value_in_animation_it->component_name + ":" + value_in_animation_it->value_name;

		//	JsonObject animation_value_section_data = animation_data.CreateSubJsonObject(section_name + "-AnimationValueSection");

		//	animation_value_section_data.SetData((int)animation_value_section.value_type, "ValueType");

		//	for (int i = 0; i < animation_value_section.animation_key_frames.size(); ++i)
		//	{
		//		const AnimationKeyFrame& key_frame = animation_value_section.animation_key_frames[i];

		//		JsonObject key_frame_data = animation_value_section_data.CreateSubJsonObject("KeyFrame " + std::to_string(i));
		//		key_frame_data.SetData(key_frame.timestamp, "Timestamp");
		//		key_frame_data.SetData((int)key_frame.value_interpolation, "Value_Interpolation");

		//		if (animation_value_section.value_type == AnimationValueType::Float)
		//		{
		//			key_frame_data.SetData(m_animation_value_storage.animation_value_float_storage[key_frame.value_data_id], "Value_Float");
		//		}
		//		else if (animation_value_section.value_type == AnimationValueType::Bool)
		//		{
		//			key_frame_data.SetData(m_animation_value_storage.animation_value_bool_storage[key_frame.value_data_id], "Value_Bool");
		//		}
		//		else if (animation_value_section.value_type == AnimationValueType::Int)
		//		{
		//			key_frame_data.SetData(m_animation_value_storage.animation_value_int_storage[key_frame.value_data_id], "Value_Int");
		//		}
		//		else if (animation_value_section.value_type == AnimationValueType::Vector2)
		//		{
		//			key_frame_data.SetData(m_animation_value_storage.animation_value_vector2_storage[key_frame.value_data_id], "Value_Vector2");
		//		}
		//	}
		//}
	}

	OutputFile file(animation_file_name, OutputFile::FileMode::WRITE);
	const std::string json_string = save_animation.GetJsonString();
	file.Write((uint32_t)json_string.size());
	file.Write((void*)json_string.c_str(), (uint32_t)json_string.size());
	file.Close();

	std::cout << save_animation.GetJsonString() << "\n";
}

void AnimatorHandler::LoadAnimation(const std::string& folder_path, const std::string& animation_file_name)
{
	ClearAnimationData();
	EntityManager* entity_manager = SceneManager::GetSceneManager()->GetEntityManager(SceneManager::GetActiveSceneIndex());
	const auto& children = SceneHierarchy::Get()->GetAllChildren(entity_manager->GetSceneIndex(), m_animation_base_entity);
	for (const Entity entity : children)
	{
		entity_manager->RemoveEntity(entity);

	}
	m_current_edit_entity = m_animation_base_entity;

	OutputFile file(animation_file_name, OutputFile::FileMode::READ);
	if (!file.FileExists())
	{
		return;
	}

	const uint32_t json_text_size = file.Read<uint32_t>();
	std::string json_string;
	json_string.resize(json_text_size);
	file.Read((void*)json_string.c_str(), json_text_size);
	file.Close();

	JsonObject load_animation(json_string);

	if (!load_animation.ObjectExist("AnimationSystemVersion"))
	{
		if (AnimationManager::Get()->LoadAnimation(SceneManager::GetActiveSceneIndex(), m_animation_base_entity, animation_file_name))
		{
			const auto sprite_texture_path = AnimationManager::Get()->GetAnimationTexturePath(SceneManager::GetActiveSceneIndex(), m_animation_base_entity, animation_file_name);
			m_animation_texture_name = sprite_texture_path.substr(folder_path.length(), sprite_texture_path.length() - folder_path.length());
		}
		return;
	}

	{
		JsonObject animation_helper_data = load_animation.GetSubJsonObject("Animation_Helper_Data");
		animation_helper_data.LoadData(m_split_size, "SplitSize");
		animation_helper_data.LoadData(m_max_split_index, "Splits");
		animation_helper_data.LoadData(m_time_between_splits, "TimeBetweenSplits");
	}

	JsonObject animatable_sprite_data = load_animation.GetSubJsonObject("AnimatableSpriteData");
	animatable_sprite_data.LoadData(m_animation_max_time, "AnimationTime");
	AnimatableSpriteComponentInterface::LoadAnimatableSpriteComponent(m_animation_base_entity, entity_manager, &animatable_sprite_data);

	int32_t animation_system_version = 0;
	load_animation.GetSubJsonObject("AnimationSystemVersion").LoadData(animation_system_version, "Version");

	if (animation_system_version == 1)
	{
		LoadAnimationVersion1(load_animation, folder_path);
		return;
	}

	ParentAnimationDataMap parent_animation_data_map{};
	Entity old_parent_entity;
	JsonObject hierarchy_data = load_animation.GetSubJsonObject("HierarchyData");
	hierarchy_data.LoadData(old_parent_entity, "ParentEntity");
	AnimationManager::LoadChildData(hierarchy_data, old_parent_entity, entity_manager, parent_animation_data_map, m_animation_value_storage);

	struct OldAndNewEntity
	{
		Entity old_entity;
		Entity new_entity;
	};
	std::vector<OldAndNewEntity> entities_to_process{ OldAndNewEntity{.old_entity = old_parent_entity, .new_entity = m_animation_base_entity} };
	entity_manager->GetComponent<SpriteComponent>(m_animation_base_entity) = parent_animation_data_map.at(old_parent_entity).sprite;
	while (!entities_to_process.empty())
	{
		const OldAndNewEntity old_and_new_entity = entities_to_process.back();
		entities_to_process.pop_back();
		const ParentAnimationData& parent_animation_data = parent_animation_data_map.at(old_and_new_entity.old_entity);

		for (const AnimationValueSection& animation_value_section : parent_animation_data.animation_value_sections)
		{
			const AnimationValueSectionId section_id{ .setter_id = animation_value_section.value_setter_storage_id, .entity = old_and_new_entity.new_entity };
			m_animation_value_sections.insert({ section_id, animation_value_section });

			if (!m_values_used_in_animation.contains(old_and_new_entity.new_entity))
			{
				m_values_used_in_animation.insert({ old_and_new_entity.new_entity, {} });
			}

			const auto& animation_value_setter_storages = AnimationManager::Get()->GetAnimationValueSetterStorages();
			m_values_used_in_animation.at(old_and_new_entity.new_entity).push_back(
				ValueInAnimation{ 
				.component_name = animation_value_setter_storages[section_id.setter_id].component_name, 
				.value_name = animation_value_setter_storages[section_id.setter_id].value_name, 
				.setter_id = section_id.setter_id
				});
		}

		for (const Entity child : parent_animation_data.children)
		{
			const Entity new_child_entity = entity_manager->NewEntity();
			entity_manager->AddComponent<TransformComponent>(new_child_entity);
			entity_manager->AddComponent<SpriteComponent>(new_child_entity) = parent_animation_data.sprite;
			SceneHierarchy::Get()->AddParentChildRelation(old_and_new_entity.new_entity, new_child_entity);

			entities_to_process.push_back(OldAndNewEntity{ .old_entity = child, .new_entity = new_child_entity });
		}
	}

	const TextureHandle texture_handle = entity_manager->GetComponent<SpriteComponent>(m_animation_base_entity).texture_handle;
	if (RenderCore::Get()->IsTextureLoaded(texture_handle))
	{
		m_animation_texture_name = AssetManager::Get()->GetAssetPath(RenderCore::Get()->GetTextureAssetHandle(texture_handle));
	}
}

void AnimatorHandler::LoadAnimationVersion1(JsonObject& load_animation, const std::string& folder_path)
{
	EntityManager* entity_manager = SceneManager::GetSceneManager()->GetEntityManager(SceneManager::GetActiveSceneIndex());

	{
		JsonObject sprite_data = load_animation.GetSubJsonObject("SpriteData");

		std::string sprite_texture_path;
		sprite_data.LoadData(sprite_texture_path, "Texture_Path");

		m_animation_texture_name = "";
		if (sprite_texture_path != "")
		{
			m_animation_texture_name = sprite_texture_path.substr(folder_path.length(), sprite_texture_path.length() - folder_path.length());
		}
	}

	const std::vector<AnimationValueSection> animation_sections = AnimationManager::LoadAnimationDataAndGetAnimationValueSections(entity_manager, m_animation_base_entity, load_animation, m_animation_value_storage);

	JsonObject animation_data = load_animation.GetSubJsonObject("Animatable");
	int animation_section_index = 0;
	for (const std::string& object_name : animation_data.GetObjectNames())
	{
		std::string fixed_object_name = object_name.substr(0, object_name.length() - sizeof("-AnimationValueSection") + 1);
		std::string component_name = fixed_object_name.substr(0, fixed_object_name.find(":"));
		std::string value_name = fixed_object_name.substr(fixed_object_name.find(":") + 1, fixed_object_name.size());

		if (!m_values_used_in_animation.contains(m_animation_base_entity))
		{
			m_values_used_in_animation.insert({ m_animation_base_entity, {} });
		}

		m_values_used_in_animation.at(m_animation_base_entity).push_back(ValueInAnimation{ .component_name = component_name, .value_name = value_name, .setter_id = animation_sections[animation_section_index++].value_setter_storage_id });
	}

	for (const AnimationValueSection& animation_section : animation_sections)
	{
		const AnimationValueSectionId animation_value_section_id{ .setter_id = animation_section.value_setter_storage_id , .entity = m_animation_base_entity };
		m_animation_value_sections.insert({ animation_value_section_id , animation_section });
	}
}

void AnimatorHandler::ClearAnimationData()
{
	m_values_used_in_animation.clear();
	m_animation_value_sections.clear();
	m_animation_value_storage.animation_value_float_storage.clear();
	m_animation_value_storage.animation_value_bool_storage.clear();
	m_animation_value_storage.animation_value_int_storage.clear();
	m_animation_value_storage.animation_value_vector2_storage.clear();
}
