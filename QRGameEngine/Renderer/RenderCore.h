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
#include "SceneSystem/SceneDefines.h"

class DX12StackAllocator;
class ImGUIMain;

class Scene;

class RenderCore
{
	friend ImGUIMain;

private:
	struct SpriteData
	{
		uint32_t GPU_texture_view_handle;
		Vector2 uv[4];
		float pad[3];
	};

	struct TextureHandleData
	{
		DX12TextureHandle texture_internal_handle;
		DX12TextureViewHandle texture_internal_view_handle;

		bool operator==(const TextureHandleData& other) const
		{
			return texture_internal_handle == other.texture_internal_handle && texture_internal_view_handle == other.texture_internal_view_handle;
		}
	};
	struct VertexGrid
	{
		float position[3];
		float pad;
	};

	struct SubscribeToTextureLoading
	{
		SceneIndex scene_index;
		Entity entity;
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

	qr::unordered_map<AssetHandle, TextureHandle> m_asset_to_texture;
	qr::unordered_map<TextureHandle, AssetHandle> m_texture_to_asset;
	qr::unordered_map<TextureHandle, TextureHandleData> m_texture_handles;

	qr::unordered_map<TextureHandle, std::vector<SubscribeToTextureLoading>> m_subscribe_to_texture_loading;

	DX12RootSignature m_grid_root_signature;
	DX12Pipeline m_grid_pipeline;
	DX12BufferHandle m_editor_lines_handle;
	DX12BufferViewHandle m_editor_lines_view_handle;
	uint64_t m_editor_lines_amount = 0;

	DX12BufferHandle m_line_color_buffer;
	DX12BufferViewHandle m_line_color_buffer_view;

	std::vector<VertexGrid> m_debug_lines;

	static RenderCore* s_render_core;

	TextureHandle m_texture_handle_counter = 0;

	TextureHandleData m_solid_color_texture;

private:
	DX12Core* GetDX12Core();

	static void AssetFinishedLoadingListenEvent(AssetHandle asset_handle);
	void LoadTextureWithAssetHandle(AssetHandle asset_handle);

public:
	RenderCore(uint32_t window_width, uint32_t window_height, const std::wstring& window_name);
	~RenderCore();

	bool UpdateRender(Scene* draw_scene);

	TextureHandle LoadTexture(const std::string& texture_file_name);
	bool IsTextureAvailable(TextureHandle texture_handle);
	bool IsTextureLoaded(TextureHandle texture_handle);
	void SubscribeEntityToTextureLoading(const TextureHandle texture_handle, const SceneIndex scene_index, const Entity entity);

	AssetHandle GetTextureAssetHandle(TextureHandle texture_handle);
	DX12TextureViewHandle GetTextureViewHandle(TextureHandle texture_handle);
	
	void AddLine(const Vector2& line);

	void Resize(UINT window_width, UINT window_height);

	static RenderCore* Get();

	Window* GetWindow();
};

