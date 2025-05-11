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
		m_draw_scene->SetSelectEntity(base_ent);
		return true;
	}

	const AnimationUIData animationUIData = AnimationUI();
	SpriteComponent& sprite = ent_man->GetComponent<SpriteComponent>(m_animation_base_entity);

	sprite.uv[0] = animationUIData.uv_1;
	sprite.uv[1] = Vector2(animationUIData.uv_4.x, animationUIData.uv_1.y);
	sprite.uv[2] = Vector2(animationUIData.uv_1.x, animationUIData.uv_4.y);
	sprite.uv[3] = animationUIData.uv_4;

	if (animationUIData.back_pressed)
	{
		ent_man->RemoveEntity(m_animation_base_entity);
		m_animation_base_entity = NULL_ENTITY;
		m_draw_scene->Load(m_animation_temp_save_file_name);
		return false;
	}

	const std::string folder_path = "../QRGameEngine/Textures/";
	const std::string texture_full_path = folder_path + m_animation_texture_name;
	bool texture_exists = false;
	if (std::filesystem::exists(texture_full_path) && std::filesystem::is_regular_file(texture_full_path))
	{
		sprite.texture_handle = RenderCore::Get()->LoadTexture(texture_full_path, SceneManager::GetActiveSceneIndex());
		texture_exists = true;
	}

	std::string fixed_animation_file_name = "Animations/";
	//So goddamn stupid!!!
	fixed_animation_file_name.insert(fixed_animation_file_name.length(), m_animation_file_name.c_str());
	fixed_animation_file_name += ".anim";

	if (animationUIData.save_animation_pressed && !texture_exists)
	{
		std::cout << "Animation Texture doesn't exists\n";
	}
	if (animationUIData.save_animation_pressed && texture_exists)
	{
		AnimationManager::Get()->SaveAnimation(SceneManager::GetActiveSceneIndex(), m_animation_base_entity, fixed_animation_file_name);
	}
	if (animationUIData.load_animation_pressed)
	{
		if (AnimationManager::Get()->LoadAnimation(SceneManager::GetActiveSceneIndex(), m_animation_base_entity, fixed_animation_file_name))
		{
			const auto sprite_texture_path = AnimationManager::Get()->GetAnimationTexturePath(SceneManager::GetActiveSceneIndex(), m_animation_base_entity, fixed_animation_file_name);
			m_animation_texture_name = sprite_texture_path.substr(folder_path.length(), sprite_texture_path.length() - folder_path.length());
		}
	}
	return true;
}

ImGuiWindowFlags_ flag = ImGuiWindowFlags_::ImGuiWindowFlags_None;

AnimatorHandler::AnimationUIData AnimatorHandler::AnimationUI()
{
	auto* ent_man = SceneManager::GetSceneManager()->GetScene(SceneManager::GetActiveSceneIndex())->GetEntityManager();

	bool back_pressed = false;
	bool save_animation_pressed = false;
	bool load_animation_pressed = false;
	const SpriteComponent& sprite = ent_man->GetComponent<SpriteComponent>(m_animation_base_entity);
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

		ImGui::InputText("Component Name", (char*)m_component_name.c_str(), m_component_name.size());
		ImGui::SameLine();
		std::string fixed_component_name = m_component_name;
		fixed_component_name.erase(std::remove(fixed_component_name.begin(), fixed_component_name.end(), 0), fixed_component_name.end());
		if (ImGui::Button("Add Component"))
		{
			if (ent_man->ComponentExists(fixed_component_name) && !ent_man->HasComponentName(m_animation_base_entity, fixed_component_name))
			{
				ent_man->AddComponent(fixed_component_name, m_animation_base_entity);
			}
		}

		if (ImGui::BeginCombo("##Edit Component", m_edit_component_name.c_str()))
		{
			for (const auto& component_name : ent_man->GetComponentNameList(m_animation_base_entity))
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

		const auto draw_list = ImGui::GetWindowDrawList();
		auto cursor_pos = ImGui::GetCursorScreenPos();
		const auto window_pos = ImGui::GetWindowPos();
		auto canvas_size = ImGui::GetContentRegionAvail();

		draw_list->AddRectFilled(cursor_pos, ImVec2(cursor_pos.x + canvas_size.x, cursor_pos.y + canvas_size.y), ImColor(120, 120, 255, 120));

		for (float x = 0.0f; x < canvas_size.x; x += 50.0f)
		{
			draw_list->AddLine(ImVec2(cursor_pos.x + x, cursor_pos.y), ImVec2(cursor_pos.x + x, cursor_pos.y + canvas_size.y), ImColor(255, 255, 255, 255));
		}

		for (float y = 0.0f; y < canvas_size.y; y += 50.0f)
		{
			draw_list->AddLine(ImVec2(cursor_pos.x, cursor_pos.y + y), ImVec2(cursor_pos.x + canvas_size.x, cursor_pos.y + y), ImColor(255, 255, 255, 255));
		}

		draw_list->AddRectFilled(cursor_pos, ImVec2(cursor_pos.x + 100.0f, cursor_pos.y + canvas_size.y), ImColor(25, 25, 25, 255));
		draw_list->AddText(ImVec2(cursor_pos.x + 5.0f, cursor_pos.y + 20.0f), ImColor(255, 255, 255, 255), "scale: x");
		draw_list->AddCircleFilled(ImVec2(cursor_pos.x + 100.0f + 25.0f, cursor_pos.y + 25.0f), 5.0f, ImColor(0, 255, 0, 255));
		draw_list->AddCircleFilled(ImVec2(cursor_pos.x + 100.0f + 75.0f, cursor_pos.y + 25.0f), 5.0f, ImColor(0, 255, 0, 255));
		draw_list->AddLine(ImVec2(cursor_pos.x + 100.0f + 25.0f, cursor_pos.y + 25.0f), ImVec2(cursor_pos.x + 100.0f + 75.0f, cursor_pos.y + 25.0f), ImColor(0, 255, 0, 255));

		for (int i = 0; i < m_values_used_in_animation.size(); ++i)
		{
			const auto& value = m_values_used_in_animation[i];
			const std::string value_text = value.component_name.substr(0, 4) + ":" + value.value_name;
			draw_list->AddText(ImVec2(cursor_pos.x + 5.0f, cursor_pos.y + 70.0f + i * 50.0f), ImColor(255, 255, 255, 255), value_text.c_str());
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

	return AnimationUIData{.back_pressed = back_pressed, .save_animation_pressed = save_animation_pressed, .load_animation_pressed = load_animation_pressed, .uv_1 = uv_1, .uv_4 = uv_4};
}

void AnimatorHandler::EditValueFromComponent()
{
	auto* ent_man = SceneManager::GetSceneManager()->GetScene(SceneManager::GetActiveSceneIndex())->GetEntityManager();

	std::string fixed_edit_component_name = m_edit_component_name;
	fixed_edit_component_name.erase(std::remove(fixed_edit_component_name.begin(), fixed_edit_component_name.end(), 0), fixed_edit_component_name.end());

	if (ent_man->ComponentExists(fixed_edit_component_name) && ent_man->HasComponentName(m_animation_base_entity, fixed_edit_component_name) && ImGui::BeginCombo("##Value From Component", m_value_from_component_name.c_str()))
	{
		JsonObject json;
		const auto save_method = SceneLoader::Get()->GetOverrideSaveComponentMethod(fixed_edit_component_name);
		if (save_method == nullptr)
		{
			return;
		}
		const auto load_method = SceneLoader::Get()->GetOverrideLoadComponentMethod(fixed_edit_component_name);
		if (load_method == nullptr)
		{
			return;
		}
		(*save_method)(m_animation_base_entity, ent_man, &json);
		(*load_method)(m_animation_base_entity, ent_man, &json);

		for (const auto& object_name : json.GetObjectNames())
		{
			const bool is_selected = (m_value_from_component_name == object_name);
			if (ImGui::Selectable(object_name.c_str(), is_selected))
			{
				m_value_from_component_name = object_name;
			}
			if (is_selected)
			{
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}

	ImGui::SameLine();
	if (ImGui::Button("Add Value"))
	{
		std::string fixed_value_from_component_name = m_value_from_component_name;
		fixed_value_from_component_name.erase(std::remove(fixed_value_from_component_name.begin(), fixed_value_from_component_name.end(), 0), fixed_value_from_component_name.end());

		bool value_already_used = false;
		for (const ValueInAnimation& value : m_values_used_in_animation)
		{
			if (value.component_name == fixed_edit_component_name && value.value_name == fixed_value_from_component_name)
			{
				value_already_used = true;
				break;
			}
		}

		if (!value_already_used)
		{
			m_values_used_in_animation.push_back(ValueInAnimation{ .component_name = fixed_edit_component_name, .value_name = fixed_value_from_component_name });
		}
	}
}
