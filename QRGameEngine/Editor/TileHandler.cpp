#include "pch.h"
#include "TileHandler.h"
#include "SceneSystem/SceneManager.h"
#include "ECS/EntityManager.h"
#include "Components/TransformComponent.h"
#include "Components/SpriteComponent.h"
#include "Components/TileComponent.h"
#include "DrawScene.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include "Input/Keyboard.h"
#include "Renderer/RenderCore.h"
#include "Asset/AssetManager.h"
#include "Renderer/ImGUIMain.h"
#include "SceneSystem/GlobalScene.h"

constexpr float TILE_PI = (float)M_PI;
constexpr float TILE_PI_2 = (float)M_PI_2;

TileHandler::TileHandler()
{
	m_full_tile_texture_name.resize(50);
	m_empty_tile_texture_name.resize(50);
	m_output_tile_texture_name.resize(50);
}

void TileHandler::Update()
{
	bool generate_pressed = false;
	ImGui::Begin("Tile Handler");
	{
		ImGui::InputText("Full Tile Texture Name", (char*)m_full_tile_texture_name.c_str(), m_full_tile_texture_name.size());
		ImGui::InputText("Empty Tile Texture Name", (char*)m_empty_tile_texture_name.c_str(), m_empty_tile_texture_name.size());
		ImGui::InputText("Output Tile Texture Name", (char*)m_output_tile_texture_name.c_str(), m_output_tile_texture_name.size());
		ImGui::InputInt("Edge Width", &m_edge_width);
		generate_pressed = ImGui::Button("Generate Tile");
	}
	ImGui::End();

	if (generate_pressed)
	{
		std::string full_tile_path = "../QRGameEngine/Textures/";// +m_full_tile_texture_name;
		full_tile_path.insert(full_tile_path.size(), m_full_tile_texture_name.c_str());
		std::string empty_tile_path = "../QRGameEngine/Textures/";// +m_empty_tile_texture_name;
		empty_tile_path.insert(empty_tile_path.size(), m_empty_tile_texture_name.c_str());
		std::string output_tile_path = "../QRGameEngine/Textures/";// +m_output_tile_texture_name;
		output_tile_path.insert(output_tile_path.size(), m_output_tile_texture_name.c_str());

		full_tile_path.erase(std::remove(full_tile_path.begin(), full_tile_path.end(), 0), full_tile_path.end());
		empty_tile_path.erase(std::remove(empty_tile_path.begin(), empty_tile_path.end(), 0), empty_tile_path.end());
		output_tile_path.erase(std::remove(output_tile_path.begin(), output_tile_path.end(), 0), output_tile_path.end());

		const TextureHandle full_tile_texture_handle = RenderCore::Get()->ForceLoadTexture(full_tile_path, GlobalScene::Get()->GetSceneIndex());
		const TextureHandle empty_tile_texture_handle = RenderCore::Get()->ForceLoadTexture(empty_tile_path, GlobalScene::Get()->GetSceneIndex());

		TextureInfo* tile = RenderCore::Get()->GenerateTile(full_tile_texture_handle, empty_tile_texture_handle, m_tiles_per_row, m_edge_width);
		AssetManager::Get()->ExportTextureDataToPNG(tile, output_tile_path);
		free(tile->texture_data);
		delete tile;

		RenderCore::Get()->DeleteTextureHandle(full_tile_texture_handle);
		RenderCore::Get()->DeleteTextureHandle(empty_tile_texture_handle);
	}
}

void TileHandler::SetTileSprite(const Vector3& world_mouse_position, const Entity tile_entity)
{
	SceneManager* scene_manager = SceneManager::GetSceneManager();
	EntityManager* entity_manager = scene_manager->GetEntityManager(scene_manager->GetActiveSceneIndex());

	Vector3 left_tile_position = world_mouse_position + Vector3(-1.0f, 0.0f, 0.0f);
	Vector3 right_tile_position = world_mouse_position + Vector3(1.0f, 0.0f, 0.0f);
	Vector3 up_tile_position = world_mouse_position + Vector3(0.0f, 1.0f, 0.0f);
	Vector3 down_tile_position = world_mouse_position + Vector3(0.0f, -1.0f, 0.0f);
	Vector3 left_down_tile_position = world_mouse_position + Vector3(-1.0f, -1.0f, 0.0f);
	Vector3 right_down_tile_position = world_mouse_position + Vector3(1.0f, -1.0f, 0.0f);
	Vector3 left_up_tile_position = world_mouse_position + Vector3(-1.0f, 1.0f, 0.0f);
	Vector3 right_up_tile_position = world_mouse_position + Vector3(1.0f, 1.0f, 0.0f);

	const auto has_tile_component = [&](const Vector3& tile_position) -> bool
		{
			if (const auto blocks = m_draw_scene->GetBlocksFromPosition(tile_position))
			{
				for (auto block : *blocks)
				{
					if (entity_manager->HasComponent<TileComponent>(block.second.block_entity))
					{
						return true;	
					}
				}
			}

			return false;
		};

	const uint32_t Full = 0;
	const uint32_t Left = 1;
	const uint32_t Right = 2;
	const uint32_t Up = 4;
	const uint32_t Down = 8;
	const uint32_t LeftDown = 16;
	const uint32_t LeftUp = 32;
	const uint32_t RightDown = 64;
	const uint32_t RightUp = 128;

	uint32_t tile_count = Full;
	tile_count += has_tile_component(left_tile_position) ? Left : Full;
	tile_count += has_tile_component(right_tile_position) ? Right : Full;
	tile_count += has_tile_component(up_tile_position) ? Up : Full;
	tile_count += has_tile_component(down_tile_position) ? Down : Full;

	tile_count += has_tile_component(left_tile_position) && has_tile_component(down_tile_position) && has_tile_component(left_down_tile_position) ? LeftDown : Full;
	tile_count += has_tile_component(left_tile_position) && has_tile_component(up_tile_position) && has_tile_component(left_up_tile_position) ? LeftUp : Full;
	tile_count += has_tile_component(right_tile_position) && has_tile_component(down_tile_position) && has_tile_component(right_down_tile_position) ? RightDown : Full;
	tile_count += has_tile_component(right_tile_position) && has_tile_component(up_tile_position) && has_tile_component(right_up_tile_position) ? RightUp : Full;

	TransformComponent& transform_component = entity_manager->GetComponent<TransformComponent>(tile_entity);
	SpriteComponent& sprite_component = entity_manager->GetComponent<SpriteComponent>(tile_entity);

	switch (tile_count)
	{
		//1
	case Full:
		sprite_component.uv[0] = Vector2(m_uv_step * 3.0f, 0.0f);
		sprite_component.uv[1] = Vector2(m_uv_step * 4.0f, 0.0f);
		sprite_component.uv[2] = Vector2(m_uv_step * 3.0f, m_uv_step);
		sprite_component.uv[3] = Vector2(m_uv_step * 4.0f, m_uv_step);
		break;

		//2
	case Left:
		sprite_component.uv[0] = Vector2(m_uv_step * 2.0f, 0.0f);
		sprite_component.uv[1] = Vector2(m_uv_step * 3.0f, 0.0f);
		sprite_component.uv[2] = Vector2(m_uv_step * 2.0f, m_uv_step);
		sprite_component.uv[3] = Vector2(m_uv_step * 3.0f, m_uv_step);
		transform_component.SetRotation(Vector3(0.0f, 0.0f, -TILE_PI_2));
		break;
	case Right:
		sprite_component.uv[0] = Vector2(m_uv_step * 2.0f, 0.0f);
		sprite_component.uv[1] = Vector2(m_uv_step * 3.0f, 0.0f);
		sprite_component.uv[2] = Vector2(m_uv_step * 2.0f, m_uv_step);
		sprite_component.uv[3] = Vector2(m_uv_step * 3.0f, m_uv_step);
		transform_component.SetRotation(Vector3(0.0f, 0.0f, TILE_PI_2));
		break;
	case Up:
		sprite_component.uv[0] = Vector2(m_uv_step * 2.0f, 0.0f);
		sprite_component.uv[1] = Vector2(m_uv_step * 3.0f, 0.0f);
		sprite_component.uv[2] = Vector2(m_uv_step * 2.0f, m_uv_step);
		sprite_component.uv[3] = Vector2(m_uv_step * 3.0f, m_uv_step);
		transform_component.SetRotation(Vector3(0.0f, 0.0f, TILE_PI));
		break;
	case Down:
		sprite_component.uv[0] = Vector2(m_uv_step * 2.0f, 0.0f);
		sprite_component.uv[1] = Vector2(m_uv_step * 3.0f, 0.0f);
		sprite_component.uv[2] = Vector2(m_uv_step * 2.0f, m_uv_step);
		sprite_component.uv[3] = Vector2(m_uv_step * 3.0f, m_uv_step);
		transform_component.SetRotation(Vector3(0.0f, 0.0f, 0.0f));
		break;

		//3
	case Left + Right:
		sprite_component.uv[0] = Vector2(m_uv_step, 0.0f);
		sprite_component.uv[1] = Vector2(m_uv_step * 2.0f, 0.0f);
		sprite_component.uv[2] = Vector2(m_uv_step, m_uv_step);
		sprite_component.uv[3] = Vector2(m_uv_step * 2.0f, m_uv_step);
		transform_component.SetRotation(Vector3(0.0f, 0.0f, TILE_PI_2));
		break;
	case Up + Down:
		sprite_component.uv[0] = Vector2(m_uv_step, 0.0f);
		sprite_component.uv[1] = Vector2(m_uv_step * 2.0f, 0.0f);
		sprite_component.uv[2] = Vector2(m_uv_step, m_uv_step);
		sprite_component.uv[3] = Vector2(m_uv_step * 2.0f, m_uv_step);
		transform_component.SetRotation(Vector3(0.0f, 0.0f, 0.0f));
		break;

		//4
	case Left + Up:
		sprite_component.uv[0] = Vector2(0.0f, 0.0f);
		sprite_component.uv[1] = Vector2(m_uv_step, 0.0f);
		sprite_component.uv[2] = Vector2(0.0f, m_uv_step);
		sprite_component.uv[3] = Vector2(m_uv_step, m_uv_step);
		transform_component.SetRotation(Vector3(0.0f, 0.0f, TILE_PI));
		break;
	case Right + Up:
		sprite_component.uv[0] = Vector2(0.0f, 0.0f);
		sprite_component.uv[1] = Vector2(m_uv_step, 0.0f);
		sprite_component.uv[2] = Vector2(0.0f, m_uv_step);
		sprite_component.uv[3] = Vector2(m_uv_step, m_uv_step);
		transform_component.SetRotation(Vector3(0.0f, 0.0f, TILE_PI_2));
		break;
	case Right + Down:
		sprite_component.uv[0] = Vector2(0.0f, 0.0f);
		sprite_component.uv[1] = Vector2(m_uv_step, 0.0f);
		sprite_component.uv[2] = Vector2(0.0f, m_uv_step);
		sprite_component.uv[3] = Vector2(m_uv_step, m_uv_step);
		transform_component.SetRotation(Vector3(0.0f, 0.0f, 0.0f));
		break;
	case Left + Down:
		sprite_component.uv[0] = Vector2(0.0f, 0.0f);
		sprite_component.uv[1] = Vector2(m_uv_step, 0.0f);
		sprite_component.uv[2] = Vector2(0.0f, m_uv_step);
		sprite_component.uv[3] = Vector2(m_uv_step, m_uv_step);
		transform_component.SetRotation(Vector3(0.0f, 0.0f, -TILE_PI_2));
		break;

		//5
	case Left + Up + LeftUp:
		sprite_component.uv[0] = Vector2(m_uv_step * 3.0f, m_uv_step);
		sprite_component.uv[1] = Vector2(m_uv_step * 4.0f, m_uv_step);
		sprite_component.uv[2] = Vector2(m_uv_step * 3.0f, m_uv_step * 2.0f);
		sprite_component.uv[3] = Vector2(m_uv_step * 4.0f, m_uv_step * 2.0f);
		transform_component.SetRotation(Vector3(0.0f, 0.0f, TILE_PI));
		break;
	case Right + Up + RightUp:
		sprite_component.uv[0] = Vector2(m_uv_step * 3.0f, m_uv_step);
		sprite_component.uv[1] = Vector2(m_uv_step * 4.0f, m_uv_step);
		sprite_component.uv[2] = Vector2(m_uv_step * 3.0f, m_uv_step * 2.0f);
		sprite_component.uv[3] = Vector2(m_uv_step * 4.0f, m_uv_step * 2.0f);
		transform_component.SetRotation(Vector3(0.0f, 0.0f, TILE_PI_2));
		break;
	case Right + Down + RightDown:
		sprite_component.uv[0] = Vector2(m_uv_step * 3.0f, m_uv_step);
		sprite_component.uv[1] = Vector2(m_uv_step * 4.0f, m_uv_step);
		sprite_component.uv[2] = Vector2(m_uv_step * 3.0f, m_uv_step * 2.0f);
		sprite_component.uv[3] = Vector2(m_uv_step * 4.0f, m_uv_step * 2.0f);
		transform_component.SetRotation(Vector3(0.0f, 0.0f, 0.0f));
		break;
	case Left + Down + LeftDown:
		sprite_component.uv[0] = Vector2(m_uv_step * 3.0f, m_uv_step);
		sprite_component.uv[1] = Vector2(m_uv_step * 4.0f, m_uv_step);
		sprite_component.uv[2] = Vector2(m_uv_step * 3.0f, m_uv_step * 2.0f);
		sprite_component.uv[3] = Vector2(m_uv_step * 4.0f, m_uv_step * 2.0f);
		transform_component.SetRotation(Vector3(0.0f, 0.0f, -TILE_PI_2));
		break;

		//6
	case Left + Up + Down:
		sprite_component.uv[0] = Vector2(m_uv_step * 2.0f, m_uv_step);
		sprite_component.uv[1] = Vector2(m_uv_step * 3.0f, m_uv_step);
		sprite_component.uv[2] = Vector2(m_uv_step * 2.0f, m_uv_step * 2.0f);
		sprite_component.uv[3] = Vector2(m_uv_step * 3.0f, m_uv_step * 2.0f);
		transform_component.SetRotation(Vector3(0.0f, 0.0f, TILE_PI));
		break;
	case Left + Right + Up:
		sprite_component.uv[0] = Vector2(m_uv_step * 2.0f, m_uv_step);
		sprite_component.uv[1] = Vector2(m_uv_step * 3.0f, m_uv_step);
		sprite_component.uv[2] = Vector2(m_uv_step * 2.0f, m_uv_step * 2.0f);
		sprite_component.uv[3] = Vector2(m_uv_step * 3.0f, m_uv_step * 2.0f);
		transform_component.SetRotation(Vector3(0.0f, 0.0f, TILE_PI_2));
		break;
	case Right + Up + Down:
		sprite_component.uv[0] = Vector2(m_uv_step * 2.0f, m_uv_step);
		sprite_component.uv[1] = Vector2(m_uv_step * 3.0f, m_uv_step);
		sprite_component.uv[2] = Vector2(m_uv_step * 2.0f, m_uv_step * 2.0f);
		sprite_component.uv[3] = Vector2(m_uv_step * 3.0f, m_uv_step * 2.0f);
		transform_component.SetRotation(Vector3(0.0f, 0.0f, 0.0f));
		break;
	case Left + Right + Down:
		sprite_component.uv[0] = Vector2(m_uv_step * 2.0f, m_uv_step);
		sprite_component.uv[1] = Vector2(m_uv_step * 3.0f, m_uv_step);
		sprite_component.uv[2] = Vector2(m_uv_step * 2.0f, m_uv_step * 2.0f);
		sprite_component.uv[3] = Vector2(m_uv_step * 3.0f, m_uv_step * 2.0f);
		transform_component.SetRotation(Vector3(0.0f, 0.0f, -TILE_PI_2));
		break;

		//7
	case Left + Right + Up + Down:
		sprite_component.uv[0] = Vector2(m_uv_step, m_uv_step);
		sprite_component.uv[1] = Vector2(m_uv_step * 2.0f, m_uv_step);
		sprite_component.uv[2] = Vector2(m_uv_step, m_uv_step * 2.0f);
		sprite_component.uv[3] = Vector2(m_uv_step * 2.0f, m_uv_step * 2.0f);
		break;

		//8
	case Left + Right + Up + LeftUp:
		sprite_component.uv[0] = Vector2(0.0f, m_uv_step);
		sprite_component.uv[1] = Vector2(m_uv_step, m_uv_step);
		sprite_component.uv[2] = Vector2(0.0f, m_uv_step * 2.0f);
		sprite_component.uv[3] = Vector2(m_uv_step, m_uv_step * 2.0f);
		transform_component.SetRotation(Vector3(0.0f, 0.0f, TILE_PI));
		break;
	case Right + Up + Down + RightUp:
		sprite_component.uv[0] = Vector2(0.0f, m_uv_step);
		sprite_component.uv[1] = Vector2(m_uv_step, m_uv_step);
		sprite_component.uv[2] = Vector2(0.0f, m_uv_step * 2.0f);
		sprite_component.uv[3] = Vector2(m_uv_step, m_uv_step * 2.0f);
		transform_component.SetRotation(Vector3(0.0f, 0.0f, TILE_PI_2));
		break;
	case Left + Right + Down + RightDown:
		sprite_component.uv[0] = Vector2(0.0f, m_uv_step);
		sprite_component.uv[1] = Vector2(m_uv_step, m_uv_step);
		sprite_component.uv[2] = Vector2(0.0f, m_uv_step * 2.0f);
		sprite_component.uv[3] = Vector2(m_uv_step, m_uv_step * 2.0f);
		transform_component.SetRotation(Vector3(0.0f, 0.0f, 0.0f));
		break;
	case Left + Up + Down + LeftDown:
		sprite_component.uv[0] = Vector2(0.0f, m_uv_step);
		sprite_component.uv[1] = Vector2(m_uv_step, m_uv_step);
		sprite_component.uv[2] = Vector2(0.0f, m_uv_step * 2.0f);
		sprite_component.uv[3] = Vector2(m_uv_step, m_uv_step * 2.0f);
		transform_component.SetRotation(Vector3(0.0f, 0.0f, -TILE_PI_2));
		break;

		//9
	case Left + Down + Up + LeftUp:
		sprite_component.uv[0] = Vector2(m_uv_step * 3.0f, m_uv_step * 2.0f);
		sprite_component.uv[1] = Vector2(m_uv_step * 4.0f, m_uv_step * 2.0f);
		sprite_component.uv[2] = Vector2(m_uv_step * 3.0f, m_uv_step * 3.0f);
		sprite_component.uv[3] = Vector2(m_uv_step * 4.0f, m_uv_step * 3.0f);
		transform_component.SetRotation(Vector3(0.0f, 0.0f, TILE_PI));
		break;
	case Left + Up + Right + RightUp:
		sprite_component.uv[0] = Vector2(m_uv_step * 3.0f, m_uv_step * 2.0f);
		sprite_component.uv[1] = Vector2(m_uv_step * 4.0f, m_uv_step * 2.0f);
		sprite_component.uv[2] = Vector2(m_uv_step * 3.0f, m_uv_step * 3.0f);
		sprite_component.uv[3] = Vector2(m_uv_step * 4.0f, m_uv_step * 3.0f);
		transform_component.SetRotation(Vector3(0.0f, 0.0f, TILE_PI_2));
		break;
	case Right + Up + Down + RightDown:
		sprite_component.uv[0] = Vector2(m_uv_step * 3.0f, m_uv_step * 2.0f);
		sprite_component.uv[1] = Vector2(m_uv_step * 4.0f, m_uv_step * 2.0f);
		sprite_component.uv[2] = Vector2(m_uv_step * 3.0f, m_uv_step * 3.0f);
		sprite_component.uv[3] = Vector2(m_uv_step * 4.0f, m_uv_step * 3.0f);
		transform_component.SetRotation(Vector3(0.0f, 0.0f, 0.0f));
		break;
	case Left + Right + Down + LeftDown:
		sprite_component.uv[0] = Vector2(m_uv_step * 3.0f, m_uv_step * 2.0f);
		sprite_component.uv[1] = Vector2(m_uv_step * 4.0f, m_uv_step * 2.0f);
		sprite_component.uv[2] = Vector2(m_uv_step * 3.0f, m_uv_step * 3.0f);
		sprite_component.uv[3] = Vector2(m_uv_step * 4.0f, m_uv_step * 3.0f);
		transform_component.SetRotation(Vector3(0.0f, 0.0f, -TILE_PI_2));
		break;

		//10
	case Left + Right + Up + Down + LeftUp:
		sprite_component.uv[0] = Vector2(m_uv_step * 2.0f, m_uv_step * 2.0f);
		sprite_component.uv[1] = Vector2(m_uv_step * 3.0f, m_uv_step * 2.0f);
		sprite_component.uv[2] = Vector2(m_uv_step * 2.0f, m_uv_step * 3.0f);
		sprite_component.uv[3] = Vector2(m_uv_step * 3.0f, m_uv_step * 3.0f);
		transform_component.SetRotation(Vector3(0.0f, 0.0f, TILE_PI));
		break;
	case Left + Right + Up + Down + RightUp:
		sprite_component.uv[0] = Vector2(m_uv_step * 2.0f, m_uv_step * 2.0f);
		sprite_component.uv[1] = Vector2(m_uv_step * 3.0f, m_uv_step * 2.0f);
		sprite_component.uv[2] = Vector2(m_uv_step * 2.0f, m_uv_step * 3.0f);
		sprite_component.uv[3] = Vector2(m_uv_step * 3.0f, m_uv_step * 3.0f);
		transform_component.SetRotation(Vector3(0.0f, 0.0f, TILE_PI_2));
		break;
	case Left + Right + Up + Down + RightDown:
		sprite_component.uv[0] = Vector2(m_uv_step * 2.0f, m_uv_step * 2.0f);
		sprite_component.uv[1] = Vector2(m_uv_step * 3.0f, m_uv_step * 2.0f);
		sprite_component.uv[2] = Vector2(m_uv_step * 2.0f, m_uv_step * 3.0f);
		sprite_component.uv[3] = Vector2(m_uv_step * 3.0f, m_uv_step * 3.0f);
		transform_component.SetRotation(Vector3(0.0f, 0.0f, 0.0f));
		break;
	case Left + Right + Up + Down + LeftDown:
		sprite_component.uv[0] = Vector2(m_uv_step * 2.0f, m_uv_step * 2.0f);
		sprite_component.uv[1] = Vector2(m_uv_step * 3.0f, m_uv_step * 2.0f);
		sprite_component.uv[2] = Vector2(m_uv_step * 2.0f, m_uv_step * 3.0f);
		sprite_component.uv[3] = Vector2(m_uv_step * 3.0f, m_uv_step * 3.0f);
		transform_component.SetRotation(Vector3(0.0f, 0.0f, -TILE_PI_2));
		break;

		//11
	case Left + Right + Up + LeftUp + RightUp:
		sprite_component.uv[0] = Vector2(m_uv_step, m_uv_step * 2.0f);
		sprite_component.uv[1] = Vector2(m_uv_step * 2.0f, m_uv_step * 2.0f);
		sprite_component.uv[2] = Vector2(m_uv_step, m_uv_step * 3.0f);
		sprite_component.uv[3] = Vector2(m_uv_step * 2.0f, m_uv_step * 3.0f);
		transform_component.SetRotation(Vector3(0.0f, 0.0f, TILE_PI));
		break;
	case Right + Up + Down + RightUp + RightDown:
		sprite_component.uv[0] = Vector2(m_uv_step, m_uv_step * 2.0f);
		sprite_component.uv[1] = Vector2(m_uv_step * 2.0f, m_uv_step * 2.0f);
		sprite_component.uv[2] = Vector2(m_uv_step, m_uv_step * 3.0f);
		sprite_component.uv[3] = Vector2(m_uv_step * 2.0f, m_uv_step * 3.0f);
		transform_component.SetRotation(Vector3(0.0f, 0.0f, TILE_PI_2));
		break;
	case Left + Right + Down + LeftDown + RightDown:
		sprite_component.uv[0] = Vector2(m_uv_step, m_uv_step * 2.0f);
		sprite_component.uv[1] = Vector2(m_uv_step * 2.0f, m_uv_step * 2.0f);
		sprite_component.uv[2] = Vector2(m_uv_step, m_uv_step * 3.0f);
		sprite_component.uv[3] = Vector2(m_uv_step * 2.0f, m_uv_step * 3.0f);
		transform_component.SetRotation(Vector3(0.0f, 0.0f, 0.0f));
		break;
	case Left + Up + Down + LeftUp + LeftDown:
		sprite_component.uv[0] = Vector2(m_uv_step, m_uv_step * 2.0f);
		sprite_component.uv[1] = Vector2(m_uv_step * 2.0f, m_uv_step * 2.0f);
		sprite_component.uv[2] = Vector2(m_uv_step, m_uv_step * 3.0f);
		sprite_component.uv[3] = Vector2(m_uv_step * 2.0f, m_uv_step * 3.0f);
		transform_component.SetRotation(Vector3(0.0f, 0.0f, -TILE_PI_2));
		break;

		//12
	case Left + Right + Up + Down + LeftUp + RightDown:
		sprite_component.uv[0] = Vector2(0.0f, m_uv_step * 2.0f);
		sprite_component.uv[1] = Vector2(m_uv_step, m_uv_step * 2.0f);
		sprite_component.uv[2] = Vector2(0.0f, m_uv_step * 3.0f);
		sprite_component.uv[3] = Vector2(m_uv_step, m_uv_step * 3.0f);
		transform_component.SetRotation(Vector3(0.0f, 0.0f, 0.0f));
		break;
	case Left + Up + Right + Down + RightUp + LeftDown:
		sprite_component.uv[0] = Vector2(0.0f, m_uv_step * 2.0f);
		sprite_component.uv[1] = Vector2(m_uv_step, m_uv_step * 2.0f);
		sprite_component.uv[2] = Vector2(0.0f, m_uv_step * 3.0f);
		sprite_component.uv[3] = Vector2(m_uv_step, m_uv_step * 3.0f);
		transform_component.SetRotation(Vector3(0.0f, 0.0f, TILE_PI_2));
		break;

		//13
	case Left + Right + Down + Up + LeftUp + LeftDown:
		sprite_component.uv[0] = Vector2(m_uv_step * 3.0f, m_uv_step * 3.0f);
		sprite_component.uv[1] = Vector2(m_uv_step * 4.0f, m_uv_step * 3.0f);
		sprite_component.uv[2] = Vector2(m_uv_step * 3.0f, m_uv_step * 4.0f);
		sprite_component.uv[3] = Vector2(m_uv_step * 4.0f, m_uv_step * 4.0f);
		transform_component.SetRotation(Vector3(0.0f, 0.0f, TILE_PI));
		break;
	case Left + Right + Down + Up + LeftUp + RightUp:
		sprite_component.uv[0] = Vector2(m_uv_step * 3.0f, m_uv_step * 3.0f);
		sprite_component.uv[1] = Vector2(m_uv_step * 4.0f, m_uv_step * 3.0f);
		sprite_component.uv[2] = Vector2(m_uv_step * 3.0f, m_uv_step * 4.0f);
		sprite_component.uv[3] = Vector2(m_uv_step * 4.0f, m_uv_step * 4.0f);
		transform_component.SetRotation(Vector3(0.0f, 0.0f, TILE_PI_2));
		break;
	case Left + Right + Down + Up + RightUp + RightDown:
		sprite_component.uv[0] = Vector2(m_uv_step * 3.0f, m_uv_step * 3.0f);
		sprite_component.uv[1] = Vector2(m_uv_step * 4.0f, m_uv_step * 3.0f);
		sprite_component.uv[2] = Vector2(m_uv_step * 3.0f, m_uv_step * 4.0f);
		sprite_component.uv[3] = Vector2(m_uv_step * 4.0f, m_uv_step * 4.0f);
		transform_component.SetRotation(Vector3(0.0f, 0.0f, 0.0f));
		break;
	case Left + Right + Down + Up + LeftDown + RightDown:
		sprite_component.uv[0] = Vector2(m_uv_step * 3.0f, m_uv_step * 3.0f);
		sprite_component.uv[1] = Vector2(m_uv_step * 4.0f, m_uv_step * 3.0f);
		sprite_component.uv[2] = Vector2(m_uv_step * 3.0f, m_uv_step * 4.0f);
		sprite_component.uv[3] = Vector2(m_uv_step * 4.0f, m_uv_step * 4.0f);
		transform_component.SetRotation(Vector3(0.0f, 0.0f, -TILE_PI_2));
		break;

		//14
	case Left + Right + Down + Up + LeftUp + LeftDown + RightDown:
		sprite_component.uv[0] = Vector2(m_uv_step * 2.0f, m_uv_step * 3.0f);
		sprite_component.uv[1] = Vector2(m_uv_step * 3.0f, m_uv_step * 3.0f);
		sprite_component.uv[2] = Vector2(m_uv_step * 2.0f, m_uv_step * 4.0f);
		sprite_component.uv[3] = Vector2(m_uv_step * 3.0f, m_uv_step * 4.0f);
		transform_component.SetRotation(Vector3(0.0f, 0.0f, TILE_PI));
		break;
	case Left + Right + Down + Up + LeftUp + LeftDown + RightUp:
		sprite_component.uv[0] = Vector2(m_uv_step * 2.0f, m_uv_step * 3.0f);
		sprite_component.uv[1] = Vector2(m_uv_step * 3.0f, m_uv_step * 3.0f);
		sprite_component.uv[2] = Vector2(m_uv_step * 2.0f, m_uv_step * 4.0f);
		sprite_component.uv[3] = Vector2(m_uv_step * 3.0f, m_uv_step * 4.0f);
		transform_component.SetRotation(Vector3(0.0f, 0.0f, TILE_PI_2));
		break;
	case Left + Right + Down + Up + LeftUp + RightUp + RightDown:
		sprite_component.uv[0] = Vector2(m_uv_step * 2.0f, m_uv_step * 3.0f);
		sprite_component.uv[1] = Vector2(m_uv_step * 3.0f, m_uv_step * 3.0f);
		sprite_component.uv[2] = Vector2(m_uv_step * 2.0f, m_uv_step * 4.0f);
		sprite_component.uv[3] = Vector2(m_uv_step * 3.0f, m_uv_step * 4.0f);
		transform_component.SetRotation(Vector3(0.0f, 0.0f, 0.0f));
		break;
	case Left + Right + Down + Up + LeftDown + RightUp + RightDown:
		sprite_component.uv[0] = Vector2(m_uv_step * 2.0f, m_uv_step * 3.0f);
		sprite_component.uv[1] = Vector2(m_uv_step * 3.0f, m_uv_step * 3.0f);
		sprite_component.uv[2] = Vector2(m_uv_step * 2.0f, m_uv_step * 4.0f);
		sprite_component.uv[3] = Vector2(m_uv_step * 3.0f, m_uv_step * 4.0f);
		transform_component.SetRotation(Vector3(0.0f, 0.0f, -TILE_PI_2));
		break;

		//15
	case Left + Right + Down + Up + LeftUp + LeftDown + RightDown + RightUp:
		sprite_component.uv[0] = Vector2(m_uv_step, m_uv_step * 3.0f);
		sprite_component.uv[1] = Vector2(m_uv_step * 2.0f, m_uv_step * 3.0f);
		sprite_component.uv[2] = Vector2(m_uv_step, m_uv_step * 4.0f);
		sprite_component.uv[3] = Vector2(m_uv_step * 2.0f, m_uv_step * 4.0f);
		break;
	}
}

void TileHandler::SetNeighborTileSprites(const Vector3& world_mouse_position)
{
	SceneManager* scene_manager = SceneManager::GetSceneManager();
	EntityManager* entity_manager = scene_manager->GetEntityManager(scene_manager->GetActiveSceneIndex());

	Vector3 left_tile_position = world_mouse_position + Vector3(-1.0f, 0.0f, 0.0f);
	Vector3 right_tile_position = world_mouse_position + Vector3(1.0f, 0.0f, 0.0f);
	Vector3 up_tile_position = world_mouse_position + Vector3(0.0f, 1.0f, 0.0f);
	Vector3 down_tile_position = world_mouse_position + Vector3(0.0f, -1.0f, 0.0f);
	Vector3 left_down_tile_position = world_mouse_position + Vector3(-1.0f, -1.0f, 0.0f);
	Vector3 right_down_tile_position = world_mouse_position + Vector3(1.0f, -1.0f, 0.0f);
	Vector3 left_up_tile_position = world_mouse_position + Vector3(-1.0f, 1.0f, 0.0f);
	Vector3 right_up_tile_position = world_mouse_position + Vector3(1.0f, 1.0f, 0.0f);

	const auto set_tile_sprite = [&](const Vector3& tile_position)
		{
			if (const auto blocks = m_draw_scene->GetBlocksFromPosition(tile_position))
			{
				for (auto block : *blocks)
				{
					if (entity_manager->HasComponent<TileComponent>(block.second.block_entity))
					{
						SetTileSprite(tile_position, block.second.block_entity);
					}
				}
			}
		};

	set_tile_sprite(left_tile_position);
	set_tile_sprite(right_tile_position);
	set_tile_sprite(up_tile_position);
	set_tile_sprite(down_tile_position);

	set_tile_sprite(left_down_tile_position);
	set_tile_sprite(right_down_tile_position);
	set_tile_sprite(left_up_tile_position);
	set_tile_sprite(right_up_tile_position);
}