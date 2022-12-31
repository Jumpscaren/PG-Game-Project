#pragma once
#include "Renderer/RenderTypes.h"

struct TransformComponent
{
	DirectX::XMMATRIX world_matrix;
};

struct SpriteComponent
{
	TextureHandle texture_handle;
	struct UV
	{
		float x, y;
	} uv;
};