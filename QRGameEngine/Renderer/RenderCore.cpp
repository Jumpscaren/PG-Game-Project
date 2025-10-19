#include "pch.h"
#include "RenderCore.h"
#include "DX12CORE/DX12RootSignature.h"
#include "DX12CORE/DX12Pipeline.h"
#include "DX12CORE/DX12StackAllocator.h"
#include "SceneSystem/SceneManager.h"
#include "ECS/EntityManager.h"
#include "ImGUIMain.h"
#include "Asset/AssetManager.h"
#include "Components/CameraComponent.h"
#include "Time/Time.h"
#include "SceneSystem/GlobalScene.h"
#include "Components/AnimatableSpriteComponent.h"
#include <algorithm>
#include "Event/EventCore.h"

RenderCore* RenderCore::s_render_core = nullptr;

DX12Core* RenderCore::GetDX12Core()
{
	return &m_dx12_core;
}

void RenderCore::AssetFinishedLoadingListenEvent(AssetHandle asset_handle)
{
	if (RenderCore::s_render_core->m_asset_to_texture.contains(asset_handle))
	{
		RenderCore::s_render_core->LoadTextureWithAssetHandle(asset_handle);
	}
}

void RenderCore::LoadTextureWithAssetHandle(AssetHandle asset_handle)
{
	TextureInfo* texture_info = AssetManager::Get()->GetTextureData(asset_handle);

	DX12TextureHandle texture_internal_handle = m_dx12_core.GetTextureManager()->AddTexture(&m_dx12_core, texture_info, TextureFlags::NONE_FLAG);
	DX12TextureViewHandle texture_internal_view_handle = m_dx12_core.GetTextureManager()->AddView(&m_dx12_core, texture_internal_handle, ViewType::SHADER_RESOURCE_VIEW);

	const TextureHandle texture_handle = m_asset_to_texture.at(asset_handle);
	auto& texture_handle_data = m_texture_handles.at(texture_handle);
	texture_handle_data.texture_internal_handle = texture_internal_handle;
	texture_handle_data.texture_internal_view_handle = texture_internal_view_handle;

	AssetManager::Get()->DeleteCPUAssetDataIfGPUOnly(asset_handle);

	if (auto it = m_subscribe_to_texture_loading.find(texture_handle); it != m_subscribe_to_texture_loading.end())
	{
		for (const auto& subscribe_entity : it->second)
		{
			if (!SceneManager::GetSceneManager()->SceneExists(subscribe_entity.scene_index))
			{
				continue;
			}

			EntityManager* entity_manager = SceneManager::GetSceneManager()->GetEntityManager(subscribe_entity.scene_index);
			if (!entity_manager->EntityExists(subscribe_entity.entity) || !entity_manager->HasComponent<SpriteComponent>(subscribe_entity.entity))
			{
				continue;
			}

			entity_manager->GetComponent<SpriteComponent>(subscribe_entity.entity).texture_handle = texture_handle;
		}

		m_subscribe_to_texture_loading.erase(it);
	}
}

void RenderCore::AssetDeletedListenEvent(const AssetHandle asset_handle)
{
	if (RenderCore::s_render_core->m_asset_to_texture.contains(asset_handle))
	{
		RenderCore::s_render_core->DeleteTexture(asset_handle);
	}
}

void RenderCore::DeleteTexture(const AssetHandle asset_handle)
{
	const TextureHandle texture_handle = m_asset_to_texture.at(asset_handle);

	const TextureHandleData texture_handle_data = m_texture_handles.at(texture_handle);

	m_dx12_core.GetResourceDestroyer()->FreeTexture(&m_dx12_core, texture_handle_data.texture_internal_handle);

	m_texture_handles.erase(texture_handle);
	m_texture_to_asset.erase(texture_handle);
	m_asset_to_texture.erase(asset_handle);
}

RenderCore::RenderCore(uint32_t window_width, uint32_t window_height, const std::wstring& window_name)
{
	s_render_core = this;

	m_window = std::make_unique<Window>(window_width, window_height, window_name, window_name);
	m_dx12_core.InitCore(m_window.get(), 2);

	ImGUIMain::Init(&m_dx12_core);

	//Depht stencil
	m_depthstencil = m_dx12_core.GetTextureManager()->AddTexture(&m_dx12_core, window_width, window_height, TextureFlags::DEPTSTENCIL_DENYSHADER_FLAG);
	m_depthstencil_view = m_dx12_core.GetTextureManager()->AddView(&m_dx12_core, m_depthstencil, ViewType::DEPTH_STENCIL_VIEW);
	m_dx12_core.GetCommandList()->TransitionTextureResource(&m_dx12_core, m_depthstencil, ResourceState::DEPTH_WRITE, ResourceState::COMMON);

	struct Vertex
	{
		float position[3];
		uint32_t uv_index;
	};

	const float quad_size = 0.501f;
	Vertex quad[6] =
	{
		{{-quad_size, -quad_size, 0.0f}, 2},
		{{-quad_size, quad_size, 0.0f}, 0},
		{{quad_size, quad_size, 0.0f}, 1},

		{{-quad_size, -quad_size, 0.0f}, 2},
		{{quad_size, quad_size, 0.0f}, 1},
		{{quad_size, -quad_size, 0.0f}, 3},
	};

	m_quad_handle = m_dx12_core.GetBufferManager()->AddBuffer(&m_dx12_core, &quad, sizeof(Vertex), 6, BufferType::CONSTANT_BUFFER);
	m_quad_view_handle = m_dx12_core.GetBufferManager()->AddView(&m_dx12_core, m_quad_handle, ViewType::SHADER_RESOURCE_VIEW);

	Vertex fullscreen_quad[6] =
	{
		{{-1.0f, -1.0f, 0.0f}, 2},
		{{-1.0f, 1.0f, 0.0f}, 0},
		{{1.0f, 1.0f, 0.0f}, 1},

		{{-1.0f, -1.0f, 0.0f}, 2},
		{{1.0f, 1.0f, 0.0f}, 1},
		{{1.0f, -1.0f, 0.0f}, 3},
	};

	m_fullscreen_quad_handle = m_dx12_core.GetBufferManager()->AddBuffer(&m_dx12_core, &fullscreen_quad, sizeof(Vertex), 6, BufferType::CONSTANT_BUFFER);
	m_fullscreen_quad_view_handle = m_dx12_core.GetBufferManager()->AddView(&m_dx12_core, m_quad_handle, ViewType::SHADER_RESOURCE_VIEW);

#ifdef _EDITOR
	std::vector<VertexGrid> lines;
	for (int j = -1000; j <= 1000; ++j)
	{
		lines.push_back({ {(float)(-1000), (float)(j), 2.0f}, 0.0f });
		lines.push_back({ {(float)(1000), (float)(j), 2.0f}, 0.0f });
		lines.push_back({ {(float)(j), (float)(-1000), 2.0f}, 0.0f });
		lines.push_back({ {(float)(j), (float)(1000), 2.0f}, 0.0f });
	}
	m_editor_lines_amount = lines.size();

	m_editor_lines_handle = m_dx12_core.GetBufferManager()->AddBuffer(&m_dx12_core, lines.data(), sizeof(VertexGrid), m_editor_lines_amount, BufferType::CONSTANT_BUFFER);
	m_editor_lines_view_handle = m_dx12_core.GetBufferManager()->AddView(&m_dx12_core, m_editor_lines_handle, ViewType::SHADER_RESOURCE_VIEW);
#endif

	m_camera_buffer = m_dx12_core.GetBufferManager()->AddBuffer(&m_dx12_core, sizeof(CameraComponent), 1, BufferType::MODIFIABLE_BUFFER);
	m_camera_buffer_view = m_dx12_core.GetBufferManager()->AddView(&m_dx12_core, m_camera_buffer, ViewType::SHADER_RESOURCE_VIEW);

	m_line_color_buffer = m_dx12_core.GetBufferManager()->AddBuffer(&m_dx12_core, sizeof(Vector4), 1, BufferType::MODIFIABLE_BUFFER);
	m_line_color_buffer_view = m_dx12_core.GetBufferManager()->AddView(&m_dx12_core, m_line_color_buffer, ViewType::SHADER_RESOURCE_VIEW);

	m_stack_allocator = new DX12StackAllocator(&m_dx12_core, 1'000);

	m_root_signature
		.AddStaticSampler(&m_dx12_core, SamplerTypes::POINT_WRAP, 0)
		.AddConstant(&m_dx12_core, ShaderVisibility::VERTEX, 0)
		.AddConstant(&m_dx12_core, ShaderVisibility::PIXEL, 0)
		.AddConstant(&m_dx12_core, ShaderVisibility::VERTEX, 1)
		.AddConstant(&m_dx12_core, ShaderVisibility::VERTEX, 2)
		.AddConstant(&m_dx12_core, ShaderVisibility::VERTEX, 1, 1)
		.InitRootSignature(&m_dx12_core);

	m_pipeline.InitPipeline(&m_dx12_core, &m_root_signature, L"../QRGameEngine/Shaders/VertexShader.hlsl", L"../QRGameEngine/Shaders/PixelShader.hlsl");

	//Other shader
	m_grid_root_signature
		.AddStaticSampler(&m_dx12_core, SamplerTypes::LINEAR_WRAP, 0)
		.AddConstant(&m_dx12_core, ShaderVisibility::VERTEX, 0)
		.AddConstant(&m_dx12_core, ShaderVisibility::VERTEX, 1)
		.AddConstant(&m_dx12_core, ShaderVisibility::PIXEL, 0)
		.InitRootSignature(&m_dx12_core);

	m_grid_pipeline
		.AddTopology(D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE)
		.InitPipeline(&m_dx12_core, &m_grid_root_signature, L"../QRGameEngine/Shaders/GridVS.hlsl", L"../QRGameEngine/Shaders/GridPS.hlsl");

	m_tile_generator_root_signature
		.AddStaticSampler(&m_dx12_core, SamplerTypes::POINT_WRAP, 0)
		.AddConstant(&m_dx12_core, ShaderVisibility::VERTEX, 0)
		.AddConstant(&m_dx12_core, ShaderVisibility::PIXEL, 0)
		.AddConstant(&m_dx12_core, ShaderVisibility::PIXEL, 1)
		.AddConstant(&m_dx12_core, ShaderVisibility::PIXEL, 2)
		.AddConstant(&m_dx12_core, ShaderVisibility::PIXEL, 3)
		.InitRootSignature(&m_dx12_core);

	m_tile_generator_pipeline
		.AddDepthStencil(false)
		.InitPipeline(&m_dx12_core, &m_tile_generator_root_signature, L"../QRGameEngine/Shaders/TileVertexShader.hlsl", L"../QRGameEngine/Shaders/TilePixelShader.hlsl");

	m_dx12_core.GetCommandList()->Execute(&m_dx12_core, m_dx12_core.GetGraphicsCommandQueue());
	m_dx12_core.GetCommandList()->SignalAndWait(&m_dx12_core, m_dx12_core.GetGraphicsCommandQueue());
	m_dx12_core.ResetBuffers();

	EventCore::Get()->ListenToEvent<&RenderCore::AssetFinishedLoadingListenEvent>("Asset Finished Loading", 0, &RenderCore::AssetFinishedLoadingListenEvent);
	EventCore::Get()->ListenToEvent<&RenderCore::AssetDeletedListenEvent>("DeletedAsset", 0, &RenderCore::AssetDeletedListenEvent);

	/*	unsigned char* texture_data;
	uint32_t width, height, comp, channels;*/
	unsigned char texture_data[4] = { 255, 255, 255, 255 };
	TextureInfo texture_info{ .texture_data =  texture_data, .width = 1, .height = 1, .comp = 4, .channels = 4 };
	m_solid_color_texture.texture_internal_handle = m_dx12_core.GetTextureManager()->AddTexture(&m_dx12_core, &texture_info, TextureFlags::NONE_FLAG);
	m_solid_color_texture.texture_internal_view_handle = m_dx12_core.GetTextureManager()->AddView(&m_dx12_core, m_solid_color_texture.texture_internal_handle, ViewType::SHADER_RESOURCE_VIEW);
}

RenderCore::~RenderCore()
{
	ImGUIMain::Destroy();
	delete m_stack_allocator;
}

bool RenderCore::UpdateRender(Scene* draw_scene)
{
	if (!m_dx12_core.GetCommandList()->IsCompleted(&m_dx12_core))
	{
		ImGUIMain::EndFrame();
		return m_window->WinMsg();
	}

	//Set up data
	uint32_t render_object_amount = 0;

	EntityManager* assamble_render_data_ent_man = draw_scene->GetEntityManager();
	const auto assamble_render_data = [&](const Entity entity, const TransformComponent& transform, const SpriteComponent& sprite)
		{
			SpriteData sprite_data;

			if (const auto it = m_texture_handles.find(sprite.texture_handle); it != m_texture_handles.end())
			{
				const DX12TextureViewHandle texture_view = it->second.texture_internal_view_handle;
				sprite_data.GPU_texture_view_handle = m_dx12_core.GetTextureManager()->ConvertTextureViewHandleToGPUTextureViewHandle(texture_view);
			}

			sprite_data.uv[0] = sprite.uv[sprite.uv_indicies[0]];
			sprite_data.uv[1] = sprite.uv[sprite.uv_indicies[1]];
			sprite_data.uv[2] = sprite.uv[sprite.uv_indicies[2]];
			sprite_data.uv[3] = sprite.uv[sprite.uv_indicies[3]];
			
			sprite_data.addative_color = sprite.addative_color;

			if (!sprite.show)
			{
				return;
			}

			if (render_object_amount < m_transform_data_vector.size())
			{
				m_transform_data_vector[render_object_amount] = transform.world_matrix;
				m_sprite_data_vector[render_object_amount] = sprite_data;
			}
			else
			{
				m_transform_data_vector.push_back(transform.world_matrix);
				m_sprite_data_vector.push_back(sprite_data);
			}

			++render_object_amount;
		};

	//Slow, needs to be fixed
	//Timer timer;
	draw_scene->GetEntityManager()->System<TransformComponent, SpriteComponent>(assamble_render_data);
	Scene* draw_global_scene = SceneManager::GetSceneManager()->GetScene(GlobalScene::Get()->GetSceneIndex());
	assamble_render_data_ent_man = draw_global_scene->GetEntityManager();
	draw_global_scene->GetEntityManager()->System<TransformComponent, SpriteComponent>(assamble_render_data);


	CameraComponent active_camera = {};
	float view_size = 0.0f;
	const auto assamble_camera_data = [&](CameraComponent& camera, const TransformComponent& transform)
		{
			Vector3 pos = transform.GetPosition();
			//Hardcoded camera position to 0
			float fake_camera_z_position = 0.0f;
			active_camera.view_matrix = DirectX::XMMatrixLookAtLH({ pos.x,pos.y, fake_camera_z_position }, { pos.x,pos.y, fake_camera_z_position + 1.0f }, { 0,1,0 });
			//active_camera.proj_matrix = DirectX::XMMatrixPerspectiveFovLH(DirectX::XM_PIDIV4, m_window->GetWindowHeight() / m_window->GetWindowWidth(), 0.1f, 800.0f);

			float screen_width = m_window->GetWindowWidth();
			float screen_height = m_window->GetWindowHeight();
			view_size = pos.z;
			if (view_size < 1.0f)
				view_size = 1.0f;
			active_camera.proj_matrix = DirectX::XMMatrixOrthographicLH(view_size, view_size * screen_height / screen_width, 0.1f, 1000.0f);

			camera = active_camera;

			active_camera.camera_position = pos;

		};
	draw_scene->GetEntityManager()->System<CameraComponent, TransformComponent>(assamble_camera_data);
	draw_global_scene->GetEntityManager()->System<CameraComponent, TransformComponent>(assamble_camera_data);

	//------------------------------------------------------------------------------------------------------------

	m_dx12_core.GetCommandList()->Wait(&m_dx12_core);
	m_dx12_core.GetCommandList()->Reset();

	m_dx12_core.GetResourceDestroyer()->FreeResources(&m_dx12_core);

	//Set up render
	m_dx12_core.GetCommandList()->SetShaderBindableDescriptorHeap(&m_dx12_core);
	m_dx12_core.GetCommandList()->SetRootSignature(&m_root_signature);
	m_dx12_core.GetCommandList()->SetPipeline(&m_pipeline);

	DX12TextureHandle render_target_texture = m_dx12_core.GetSwapChain()->GetBackbufferTexture();
	DX12TextureViewHandle render_target_view_handle = m_dx12_core.GetSwapChain()->GetBackbufferView();

	m_dx12_core.GetCommandList()->TransitionTextureResource(&m_dx12_core, render_target_texture, ResourceState::RENDER_TARGET, ResourceState::PRESENT);

	m_dx12_core.GetCommandList()->ClearRenderTargetView(&m_dx12_core, m_dx12_core.GetSwapChain()->GetBackbufferView());
	m_dx12_core.GetCommandList()->ClearDepthStencilView(&m_dx12_core, m_depthstencil_view);

	m_dx12_core.GetCommandList()->SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_dx12_core.GetCommandList()->SetOMRenderTargets(&m_dx12_core, m_dx12_core.GetSwapChain()->GetBackbufferView(), m_depthstencil_view);

	m_dx12_core.GetCommandList()->SetViewport((uint64_t)m_window->GetWindowWidth(), (uint64_t)m_window->GetWindowHeight());
	m_dx12_core.GetCommandList()->SetScissorRect((uint64_t)m_window->GetWindowWidth(), (uint64_t)m_window->GetWindowHeight());

	//std::sort(draw_entities.begin(), draw_entities.end(), [&](uint32_t i, uint32_t j){
	//	return (m_transform_data_vector[i].GetPosition().z < m_transform_data_vector[j].GetPosition().z);
	//	});
	//std::cout << "Time: " << timer.StopTimer()/(double)Timer::TimeTypes::Milliseconds << " ms\n";


	DX12BufferHandle transform_data_buffer = 0;
	DX12BufferHandle sprite_data_buffer = 0;

	if (render_object_amount)
	{
		transform_data_buffer = m_dx12_core.GetBufferManager()->AddBuffer(&m_dx12_core, sizeof(WorldMatrixData), render_object_amount, BufferType::CONSTANT_BUFFER);
		DX12BufferViewHandle transform_data_buffer_view = m_dx12_core.GetBufferManager()->AddView(&m_dx12_core, transform_data_buffer, ViewType::SHADER_RESOURCE_VIEW);
		m_dx12_core.GetCommandList()->SetConstantBuffer(&m_dx12_core, transform_data_buffer_view, 2);
		m_dx12_core.GetBufferManager()->UploadData(&m_dx12_core, transform_data_buffer, m_transform_data_vector.data(), sizeof(WorldMatrixData), render_object_amount);

		sprite_data_buffer = m_dx12_core.GetBufferManager()->AddBuffer(&m_dx12_core, sizeof(SpriteData), render_object_amount, BufferType::CONSTANT_BUFFER);
		DX12BufferViewHandle sprite_data_buffer_view = m_dx12_core.GetBufferManager()->AddView(&m_dx12_core, sprite_data_buffer, ViewType::SHADER_RESOURCE_VIEW);
		m_dx12_core.GetCommandList()->SetConstantBuffer(&m_dx12_core, sprite_data_buffer_view, 1);
		m_dx12_core.GetCommandList()->SetConstantBuffer(&m_dx12_core, sprite_data_buffer_view, 4);
		m_dx12_core.GetBufferManager()->UploadData(&m_dx12_core, sprite_data_buffer, m_sprite_data_vector.data(), sizeof(SpriteData), render_object_amount);
	}

	m_dx12_core.GetCommandList()->SetConstantBuffer(&m_dx12_core, m_quad_view_handle, 0);

	//Camera
	m_dx12_core.GetCommandList()->SetConstantBuffer(&m_dx12_core, m_camera_buffer_view, 3);
	m_dx12_core.GetBufferManager()->UploadData(&m_dx12_core, m_camera_buffer, &active_camera, sizeof(CameraComponent), 1);

	m_dx12_core.GetCommandList()->Draw(6, render_object_amount, 0, 0);

//#ifdef _EDITOR
	m_dx12_core.GetCommandList()->SetRootSignature(&m_grid_root_signature);
	m_dx12_core.GetCommandList()->SetPipeline(&m_grid_pipeline);
	m_dx12_core.GetCommandList()->SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);

	m_dx12_core.GetCommandList()->SetConstantBuffer(&m_dx12_core, m_editor_lines_view_handle, 0);

	//Camera
	m_dx12_core.GetCommandList()->SetConstantBuffer(&m_dx12_core, m_camera_buffer_view, 1);

	m_dx12_core.GetCommandList()->SetConstantBuffer(&m_dx12_core, m_line_color_buffer_view, 2);
	Vector4 editor_line_color(1.0f, 1.0f, 1.0f, 1.0f);
	m_dx12_core.GetBufferManager()->UploadData(&m_dx12_core, m_line_color_buffer, &editor_line_color, sizeof(Vector4), 1);

#ifdef _EDITOR
	m_dx12_core.GetCommandList()->Draw(m_editor_lines_amount, 1, 0, 0);
#endif

	//Draw Debug Lines eg. Physics Debug Lines
	if (m_debug_lines.size())
	{
		auto debug_line_buffer = m_dx12_core.GetBufferManager()->AddBuffer(&m_dx12_core, m_debug_lines.data(), sizeof(VertexGrid), m_debug_lines.size(), BufferType::CONSTANT_BUFFER);
		DX12BufferViewHandle debug_lines_view = m_dx12_core.GetBufferManager()->AddView(&m_dx12_core, debug_line_buffer, ViewType::SHADER_RESOURCE_VIEW);
		m_dx12_core.GetCommandList()->SetConstantBuffer(&m_dx12_core, debug_lines_view, 0);

		Vector4 debug_line_color(0.0f, 1.0f, 0.0f, 1.0f);
		m_dx12_core.GetBufferManager()->UploadData(&m_dx12_core, m_line_color_buffer, &debug_line_color, sizeof(Vector4), 1);

		m_dx12_core.GetCommandList()->Draw(m_debug_lines.size(), 1, 0, 0);

		m_dx12_core.GetResourceDestroyer()->FreeBuffer(&m_dx12_core, debug_line_buffer);
	}
//#endif

	if (render_object_amount)
	{
		m_dx12_core.GetResourceDestroyer()->FreeBuffer(&m_dx12_core, transform_data_buffer);
		m_dx12_core.GetResourceDestroyer()->FreeBuffer(&m_dx12_core, sprite_data_buffer);
	}

	ImGUIMain::RenderFrame(&m_dx12_core);

	m_dx12_core.GetCommandList()->TransitionTextureResource(&m_dx12_core, render_target_texture, ResourceState::PRESENT, ResourceState::RENDER_TARGET);

	m_dx12_core.GetCommandList()->Execute(&m_dx12_core, m_dx12_core.GetGraphicsCommandQueue());
	m_dx12_core.GetSwapChain()->Present();
	m_dx12_core.GetCommandList()->Signal(&m_dx12_core, m_dx12_core.GetGraphicsCommandQueue());

	m_dx12_core.EndOfFrame();

	m_debug_lines.clear();

	return m_window->WinMsg();
}

void RenderCore::LoadAndSetTexture(const std::string& texture_file_name, const SceneIndex scene_index, const Entity entity)
{
	SceneManager::GetEntityManager(scene_index)->GetComponent<SpriteComponent>(entity).texture_handle = LoadTexture(texture_file_name, scene_index);
}

TextureHandle RenderCore::LoadTexture(const std::string& texture_file_name, const SceneIndex scene_index)
{
	AssetHandle texture_asset_handle = AssetManager::Get()->LoadTextureAsset(texture_file_name, scene_index);

	if (m_asset_to_texture.contains(texture_asset_handle))
	{
		return m_asset_to_texture.find(texture_asset_handle)->second;
	}

	TextureHandle texture_handle = ++m_texture_handle_counter;
	m_texture_to_asset.insert({ texture_handle, texture_asset_handle });
	m_asset_to_texture.insert({ texture_asset_handle, texture_handle });
	if (!m_texture_handles.contains(texture_handle))
	{
		m_texture_handles.insert({ texture_handle, {m_solid_color_texture.texture_internal_handle, m_solid_color_texture.texture_internal_view_handle} });

		if (!AssetManager::Get()->IsAssetLoaded(texture_asset_handle))
		{
			return texture_handle;
		}
	}

	LoadTextureWithAssetHandle(texture_asset_handle);

	return texture_handle;
}

TextureHandle RenderCore::ForceLoadTexture(const std::string& texture_file_name, SceneIndex scene_index)
{
	AssetManager::Get()->LoadTextureAsset(texture_file_name, scene_index, false);
	return LoadTexture(texture_file_name, scene_index);
}

void RenderCore::DeleteTextureHandle(const TextureHandle texture_handle)
{
	AssetManager::Get()->DeleteAsset(GetTextureAssetHandle(texture_handle));
}

bool RenderCore::IsTextureAvailable(TextureHandle texture_handle)
{
	return m_texture_handles.contains(texture_handle);
}

bool RenderCore::IsTextureLoaded(const TextureHandle texture_handle)
{
	assert(m_texture_handles.contains(texture_handle));
	return m_texture_handles.at(texture_handle) != m_solid_color_texture;
}

void RenderCore::SubscribeEntityToTextureLoading(const TextureHandle texture_handle, const SceneIndex scene_index, const Entity entity)
{
	EntityManager* entity_manager = SceneManager::GetSceneManager()->GetEntityManager(scene_index);
	assert(entity_manager->HasComponent<SpriteComponent>(entity));

	auto it = m_subscribe_to_texture_loading.find(texture_handle);
	if (it == m_subscribe_to_texture_loading.end())
	{
		m_subscribe_to_texture_loading.insert({ texture_handle, {{ scene_index, entity }} });
		return;
	}

	it->second.push_back({ scene_index, entity });
}

bool RenderCore::IsEntitySubscribedToTextureLoading(const SceneIndex scene_index, const Entity entity)
{
	for (const auto it : m_subscribe_to_texture_loading)
	{
		const std::vector<SubscribeToTextureLoading>& subscribers = it.second;
		for (const SubscribeToTextureLoading subscriber : subscribers)
		{
			if (subscriber.entity == entity && subscriber.scene_index == scene_index)
			{
				return true;
			}
		}
	}
	return false;
}

AssetHandle RenderCore::GetTextureAssetHandle(const TextureHandle texture_handle)
{
	if (m_texture_to_asset.contains(texture_handle))
	{
		return m_texture_to_asset.find(texture_handle)->second;
	}

	assert(false);
	return 0;
}

DX12TextureViewHandle RenderCore::GetTextureViewHandle(const TextureHandle texture_handle)
{
	if (m_texture_handles.contains(texture_handle))
	{
		return m_texture_handles.find(texture_handle)->second.texture_internal_view_handle;
	}

	assert(false);
	return 0;
}

TextureInfo* RenderCore::GenerateTile(const TextureHandle tile_full_input_texture, const TextureHandle tile_empty_input_texture, const uint32_t tiles_per_row, const uint32_t edge_width)
{
	m_dx12_core.GetCommandList()->Wait(&m_dx12_core);
	m_dx12_core.GetCommandList()->Reset();

	struct VertexWithUV
	{
		Vector3 position;
		Vector2 uv;
		float pad;
	};

	TextureInfo* tile_full_input_texture_info = AssetManager::Get()->GetTextureData(GetTextureAssetHandle(tile_full_input_texture));
	TextureInfo* tile_empty_input_texture_info = AssetManager::Get()->GetTextureData(GetTextureAssetHandle(tile_empty_input_texture));

	//Check that textures are squares and that the textures are the same size
	if (!(tile_full_input_texture_info->width == tile_empty_input_texture_info->width && tile_full_input_texture_info->height == tile_empty_input_texture_info->height &&
		tile_full_input_texture_info->width == tile_full_input_texture_info->height && tile_empty_input_texture_info->width == tile_empty_input_texture_info->height))
	{
		return nullptr;
	}

	const uint32_t input_texture_width = tile_full_input_texture_info->width;
	const uint32_t input_texture_height = tile_full_input_texture_info->height;
	const uint32_t output_texture_width = input_texture_width * tiles_per_row;
	const uint32_t output_texture_height = input_texture_height * tiles_per_row;
	const uint32_t total_number_of_tiles = tiles_per_row * tiles_per_row;

	const float float_tiles_per_row = (float)tiles_per_row;

	std::vector<VertexWithUV> tile_vertices;
	for (uint32_t i = 0; i < total_number_of_tiles; ++i)
	{
		// * 2.0f because the range goes between -1.0f and 1.0f
		const float x1 = (i % tiles_per_row) * 2.0f / float_tiles_per_row;
		const float x2 = (i % tiles_per_row + 1) * 2.0f / float_tiles_per_row;
		const float y1 = (i / tiles_per_row) * 2.0f / float_tiles_per_row;
		const float y2 = (i / tiles_per_row + 1) * 2.0f / float_tiles_per_row;

		tile_vertices.push_back({ {1.0f - x1, 1.0f - y1, 0.0f}, {0.0f, 1.0f}, 0.0f });
		tile_vertices.push_back({ {1.0f - x1, 1.0f - y2, 0.0f}, {0.0f, 0.0f}, 0.0f });
		tile_vertices.push_back({ {1.0f - x2, 1.0f - y2, 0.0f}, {1.0f, 0.0f}, 0.0f });

		tile_vertices.push_back({ {1.0f - x1, 1.0f - y1, 0.0f}, {0.0f, 1.0f}, 0.0f });
		tile_vertices.push_back({ {1.0f - x2, 1.0f - y2, 0.0f}, {1.0f, 0.0f}, 0.0f });
		tile_vertices.push_back({ {1.0f - x2, 1.0f - y1, 0.0f}, {1.0f, 1.0f}, 0.0f });
	}

	DX12BufferHandle tile_vertices_handle = m_dx12_core.GetBufferManager()->AddBuffer(&m_dx12_core, tile_vertices.data(), sizeof(VertexWithUV), tile_vertices.size(), BufferType::CONSTANT_BUFFER);
	DX12BufferViewHandle tile_vertices_view_handle = m_dx12_core.GetBufferManager()->AddView(&m_dx12_core, tile_vertices_handle, ViewType::SHADER_RESOURCE_VIEW);

	DX12TextureHandle tile_texture_output_handle = m_dx12_core.GetTextureManager()->AddTexture(&m_dx12_core, output_texture_width, output_texture_height, TextureFlags::RENDER_TARGET_FLAG);
	DX12TextureViewHandle tile_texture_output_view_handle = m_dx12_core.GetTextureManager()->AddView(&m_dx12_core, tile_texture_output_handle, ViewType::RENDER_TARGET_VIEW);
	m_dx12_core.GetCommandList()->TransitionTextureResource(&m_dx12_core, tile_texture_output_handle, ResourceState::RENDER_TARGET, ResourceState::COMMON);

	m_dx12_core.GetCommandList()->SetShaderBindableDescriptorHeap(&m_dx12_core);
	m_dx12_core.GetCommandList()->SetRootSignature(&m_tile_generator_root_signature);
	m_dx12_core.GetCommandList()->SetPipeline(&m_tile_generator_pipeline);

	m_dx12_core.GetCommandList()->SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_dx12_core.GetCommandList()->SetOMRenderTargets(&m_dx12_core, tile_texture_output_view_handle);

	m_dx12_core.GetCommandList()->SetViewport(output_texture_width, output_texture_width);
	m_dx12_core.GetCommandList()->SetScissorRect(output_texture_width, output_texture_width);

	m_dx12_core.GetCommandList()->SetConstantBuffer(&m_dx12_core, tile_vertices_view_handle, 0);

	const auto it = m_texture_handles.find(tile_full_input_texture);
	const DX12TextureViewHandle texture_view = it->second.texture_internal_view_handle;
	m_dx12_core.GetCommandList()->SetConstantTexture(&m_dx12_core, texture_view, 1);

	const auto empty_it = m_texture_handles.find(tile_empty_input_texture);
	const DX12TextureViewHandle empty_texture_view = empty_it->second.texture_internal_view_handle;
	m_dx12_core.GetCommandList()->SetConstantTexture(&m_dx12_core, empty_texture_view, 2);

	m_dx12_core.GetCommandList()->SetConstant(&m_dx12_core, input_texture_width, 3);
	m_dx12_core.GetCommandList()->SetConstant(&m_dx12_core, edge_width, 4);

	m_dx12_core.GetCommandList()->Draw(total_number_of_tiles * 6, total_number_of_tiles, 0, 0);

	m_dx12_core.GetCommandList()->Signal(&m_dx12_core, m_dx12_core.GetGraphicsCommandQueue());

	TextureInfo* texture_info = m_dx12_core.GetTextureManager()->GetTextureData(&m_dx12_core, tile_texture_output_handle, ResourceState::RENDER_TARGET);

	m_dx12_core.GetResourceDestroyer()->FreeBuffer(&m_dx12_core, tile_vertices_handle);
	m_dx12_core.GetResourceDestroyer()->FreeTexture(&m_dx12_core, tile_texture_output_handle);

	return texture_info;
}

void RenderCore::AddLine(const Vector2& line)
{
	VertexGrid vertex_line;
	vertex_line.position[0] = line.x;
	vertex_line.position[1] = line.y;
	vertex_line.position[2] = 0.1f;
	m_debug_lines.push_back(vertex_line);
}

void RenderCore::Resize(const UINT window_width, const UINT window_height)
{
	const auto command_lists = m_dx12_core.GetAllCommandLists();
	for (DX12CommandList* command_list : command_lists)
	{
		command_list->Wait(&m_dx12_core);
	}

	m_dx12_core.GetCommandList()->Reset();

	m_window->SetWindowWidth(window_width);
	m_window->SetWindowHeight(window_height);

	m_dx12_core.GetSwapChain()->Resize(&m_dx12_core);

	m_dx12_core.GetResourceDestroyer()->FreeTexture(&m_dx12_core, m_depthstencil);

	m_dx12_core.GetResourceDestroyer()->FreeResources(&m_dx12_core);

	//Depht stencil
	m_depthstencil = m_dx12_core.GetTextureManager()->AddTexture(&m_dx12_core, window_width, window_height, TextureFlags::DEPTSTENCIL_DENYSHADER_FLAG);
	m_depthstencil_view = m_dx12_core.GetTextureManager()->AddView(&m_dx12_core, m_depthstencil, ViewType::DEPTH_STENCIL_VIEW);
	m_dx12_core.GetCommandList()->TransitionTextureResource(&m_dx12_core, m_depthstencil, ResourceState::DEPTH_WRITE, ResourceState::COMMON);

	m_dx12_core.GetCommandList()->Execute(&m_dx12_core, m_dx12_core.GetGraphicsCommandQueue());
	m_dx12_core.GetCommandList()->Signal(&m_dx12_core, m_dx12_core.GetGraphicsCommandQueue());
}

RenderCore* RenderCore::Get()
{
	return s_render_core;
}

Window* RenderCore::GetWindow()
{
	return m_window.get();
}
