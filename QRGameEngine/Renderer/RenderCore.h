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
		Vector4 addative_color;
		float pad[3];
	};

	using WorldMatrixData = DirectX::XMMATRIX;

	struct TextureHandleData
	{
		DX12TextureHandle texture_internal_handle;
		DX12TextureViewHandle texture_internal_view_handle;

		bool operator==(const TextureHandleData& other) const
		{
			return texture_internal_handle == other.texture_internal_handle && texture_internal_view_handle == other.texture_internal_view_handle;
		}

		uint32_t width, height;
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

	std::vector<WorldMatrixData> m_transform_data_vector;
	std::vector<SpriteData> m_sprite_data_vector;

	DX12BufferHandle m_camera_buffer;
	DX12BufferViewHandle m_camera_buffer_view;

	DX12BufferHandle m_transform_data_buffer;
	DX12BufferViewHandle m_transform_data_buffer_view;
	DX12BufferHandle m_sprite_data_buffer;
	DX12BufferViewHandle m_sprite_data_buffer_view;

	std::vector<uint32_t> m_sorted_indicies;
	DX12BufferHandle m_indicies_buffer;
	DX12BufferViewHandle m_indicies_buffer_view;

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

	DX12RootSignature m_tile_generator_root_signature;
	DX12Pipeline m_tile_generator_pipeline;

	std::vector<VertexGrid> m_debug_lines;

	static RenderCore* s_render_core;

	TextureHandle m_texture_handle_counter = 0;

	TextureHandleData m_solid_color_texture;

	static constexpr uint32_t SCENES_HANDLED_PER_FRAME = 2;
	static constexpr uint32_t MAX_RENDER_OBJECTS_PER_FRAME = SCENES_HANDLED_PER_FRAME * MAX_ENTITIES_PER_SCENE;

	struct FixedResolution
	{
		uint32_t width;
		uint32_t height;
	};
	std::optional<FixedResolution> m_fixed_resolution;

	struct FixedResolutionTexture
	{
		DX12TextureHandle fixed_resolution_texture_handle;
		DX12TextureViewHandle fixed_resolution_view_render_handle;
		DX12TextureViewHandle fixed_resolution_view_shader_handle;
	};
	std::vector<FixedResolutionTexture> m_fixed_resolution_textures;

	DX12RootSignature m_fixed_resolution_root_signature;
	DX12Pipeline m_fixed_resolution_pipeline;

	float m_pixels_per_unit = 1.0f;

private:
	DX12Core* GetDX12Core();

	static void AssetFinishedLoadingListenEvent(AssetHandle asset_handle);
	void LoadTextureWithAssetHandle(AssetHandle asset_handle);

	static void AssetDeletedListenEvent(AssetHandle asset_handle);
	void DeleteTexture(AssetHandle asset_handle);

	void Initialize();

	void CreateDepthStencil();

	void CreateMeshes();
	void CreateBuffers();

	void CreateMainPipeline();
	void CreateGridPipeline();
	void CreateTileGeneratorPipeline();
	void CreateFixedResolutionPipeline();

	void CreateEditorLines();

	void CreateWhiteTexture();

	void ConnectEvents();

public:
	RenderCore(uint32_t window_width, uint32_t window_height, const std::wstring& window_name, bool fixed_resolution, float pixels_per_unit);
	~RenderCore();

	bool UpdateRender(Scene* draw_scene);

	void LoadAndSetTexture(const std::string& texture_file_name, const SceneIndex scene_index, const Entity entity);
	TextureHandle LoadTexture(const std::string& texture_file_name, SceneIndex scene_index);
	TextureHandle ForceLoadTexture(const std::string& texture_file_name, SceneIndex scene_index);
	void DeleteTextureHandle(const TextureHandle texture_handle);

	bool IsTextureAvailable(TextureHandle texture_handle);
	bool IsTextureLoaded(TextureHandle texture_handle);
	void SubscribeEntityToTextureLoading(const TextureHandle texture_handle, const SceneIndex scene_index, const Entity entity);
	bool IsEntitySubscribedToTextureLoading(const SceneIndex scene_index, const Entity entity);

	AssetHandle GetTextureAssetHandle(TextureHandle texture_handle);
	DX12TextureViewHandle GetTextureViewHandle(TextureHandle texture_handle);

	TextureInfo* GenerateTile(const TextureHandle tile_full_input_texture, const TextureHandle tile_empty_input_texture, const uint32_t tiles_per_row, const uint32_t edge_width);
	
	void AddLine(const Vector2& line);

	void Resize(UINT window_width, UINT window_height);

	static RenderCore* Get();

	Window* GetWindow();

	Vector2u GetBackbufferSize();

	Vector2 GetFixedRenderSize() const;

	float GetPixelsPerUnit() const { return m_pixels_per_unit; }
};

