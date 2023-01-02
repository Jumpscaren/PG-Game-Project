#pragma once
#include "DX12CORE/DX12Core.h"
#include "DX12CORE/DX12RootSignature.h"
#include "DX12CORE/DX12Pipeline.h"
#include "Window.h"
#include "RenderTypes.h"
#include "EngineComponents.h"

class DX12StackAllocator;

class Scene;

class RenderCore
{
private:
	struct SpriteData
	{
		uint32_t GPU_texture_view_handle;
		struct UV
		{
			float x, y;
		} uv;
		float pad;
	};

private:
	DX12Core m_dx12_core;
	std::unique_ptr<Window> m_window;
	DX12TextureHandle m_depthstencil;
	DX12TextureViewHandle m_depthstencil_view;
	DX12RootSignature m_root_signature;
	DX12Pipeline m_pipeline;
	DX12BufferViewHandle m_quad_view_handle;
	DX12StackAllocator* m_stack_allocator;

	std::vector<TransformComponent> m_transform_data_vector;
	std::vector<SpriteData> m_sprite_data_vector;

public:
	RenderCore(uint32_t window_width, uint32_t window_height, const std::wstring& window_name);
	bool UpdateRender(Scene* draw_scene);

	TextureHandle CreateTexture(std::string texture_file_name);
};

