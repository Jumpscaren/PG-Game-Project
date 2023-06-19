#pragma once
#include "DX12CORE/DX12Core.h"
#include "DX12CORE/DX12RootSignature.h"
#include "DX12CORE/DX12Pipeline.h"
#include "Window.h"
#include "RenderTypes.h"
#include "EngineComponents.h"
#include "Components/TransformComponent.h"
#include "Components/SpriteComponent.h"
#include "Asset/AssetTypes.h"

class DX12StackAllocator;

class Scene;

class RenderCore
{
private:
	struct SpriteData
	{
		uint32_t GPU_texture_view_handle;
		Vector2 uv;
		float pad;
	};

	struct TextureHandleData
	{
		DX12TextureHandle texture_handle;
		DX12TextureViewHandle texture_view_handle;
	};

private:
	DX12Core m_dx12_core;
	std::unique_ptr<Window> m_window;
	DX12TextureHandle m_depthstencil;
	DX12TextureViewHandle m_depthstencil_view;
	DX12RootSignature m_root_signature;
	DX12Pipeline m_pipeline;
	DX12BufferHandle m_quad_handle;
	DX12BufferViewHandle m_quad_view_handle;
	DX12BufferHandle m_fullscreen_quad_handle;
	DX12BufferViewHandle m_fullscreen_quad_view_handle;
	DX12StackAllocator* m_stack_allocator;

	std::vector<TransformComponent> m_transform_data_vector;
	std::vector<SpriteData> m_sprite_data_vector;

	DX12BufferHandle m_camera_buffer;
	DX12BufferViewHandle m_camera_buffer_view;

	std::unordered_map<AssetHandle, TextureHandleData> m_texture_handles;

	DX12RootSignature m_grid_root_signature;
	DX12Pipeline m_grid_pipeline;
	DX12BufferHandle m_editor_lines_handle;
	DX12BufferViewHandle m_editor_lines_view_handle;
	uint64_t m_editor_lines_amount = 0;

	static RenderCore* s_render_core;

public:
	RenderCore(uint32_t window_width, uint32_t window_height, const std::wstring& window_name);
	~RenderCore();

	bool UpdateRender(Scene* draw_scene);

	TextureHandle CreateTexture(const std::string& texture_file_name);

	static RenderCore* Get();
};

