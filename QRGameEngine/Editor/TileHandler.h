#pragma once
#include "ECS/EntityDefinition.h"
#include "Renderer/RenderTypes.h"
#include "Common/EngineTypes.h"

class DrawScene;

class TileHandler
{
public:
	TileHandler();
	void SetDrawScene(DrawScene* draw_scene) { m_draw_scene = draw_scene; }

	void Update();

public:
	void SetTileSprite(const Vector3& world_mouse_position, const Entity tile_entity);
	void SetNeighborTileSprites(const Vector3& world_mouse_position);

private:
	DrawScene* m_draw_scene = nullptr;

	std::string m_full_tile_texture_name = "TestBlock.png";
	std::string m_empty_tile_texture_name = "TestBlockEmpty.png";
	std::string m_output_tile_texture_name = "work.png";

	const uint32_t m_tiles_per_row = 4;
	const float m_uv_step = 1.0f / (float)m_tiles_per_row;
	int m_edge_width = 0;
};

