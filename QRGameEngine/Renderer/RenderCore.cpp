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
#include "Time/Timer.h"
#include "SceneSystem/GlobalScene.h"
#include <algorithm>

RenderCore* RenderCore::s_render_core = nullptr;

DX12Core* RenderCore::GetDX12Core()
{
	return &m_dx12_core;
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
		float uv[2];
		float pad;
	};

	Vertex quad[6] =
	{
		{{-0.5f, -0.5f, 0.0f}, {0.0f, 1.0f}, 0.0f},
		{{-0.5f, 0.5f, 0.0f}, {0.0f, 0.0f}, 0.0f},
		{{0.5f, 0.5f, 0.0f}, {1.0f, 0.0f}, 0.0f},

		{{-0.5f, -0.5f, 0.0f}, {0.0f, 1.0f}, 0.0f},
		{{0.5f, 0.5f, 0.0f}, {1.0f, 0.0f}, 0.0f},
		{{0.5f, -0.5f, 0.0f}, {1.0f, 1.0f}, 0.0f},
	};

	m_quad_handle = m_dx12_core.GetBufferManager()->AddBuffer(&m_dx12_core, &quad, sizeof(Vertex), 6, BufferType::CONSTANT_BUFFER);
	m_quad_view_handle = m_dx12_core.GetBufferManager()->AddView(&m_dx12_core, m_quad_handle, ViewType::SHADER_RESOURCE_VIEW);

	Vertex fullscreen_quad[6] =
	{
		{{-1.0f, -1.0f, 0.0f}, {0.0f, 1.0f}, 0.0f},
		{{-1.0f, 1.0f, 0.0f}, {0.0f, 0.0f}, 0.0f},
		{{1.0f, 1.0f, 0.0f}, {1.0f, 0.0f}, 0.0f},

		{{-1.0f, -1.0f, 0.0f}, {0.0f, 1.0f}, 0.0f},
		{{1.0f, 1.0f, 0.0f}, {1.0f, 0.0f}, 0.0f},
		{{1.0f, -1.0f, 0.0f}, {1.0f, 1.0f}, 0.0f},
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
		.AddStaticSampler(&m_dx12_core, SamplerTypes::LINEAR_WRAP, 0)
		.AddConstant(&m_dx12_core, ShaderVisibility::VERTEX, 0)
		.AddConstant(&m_dx12_core, ShaderVisibility::PIXEL, 0)
		.AddConstant(&m_dx12_core, ShaderVisibility::VERTEX, 1)
		.AddConstant(&m_dx12_core, ShaderVisibility::VERTEX, 2)
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

	m_dx12_core.GetCommandList()->Execute(&m_dx12_core, m_dx12_core.GetGraphicsCommandQueue());
	m_dx12_core.GetCommandList()->SignalAndWait(&m_dx12_core, m_dx12_core.GetGraphicsCommandQueue());
	m_dx12_core.ResetBuffers();


}

RenderCore::~RenderCore()
{
	ImGUIMain::Destroy();
}

bool RenderCore::UpdateRender(Scene* draw_scene)
{
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

	uint32_t render_object_amount = 0;

	//Slow, needs to be fixed
	//Timer timer;
	draw_scene->GetEntityManager()->System<TransformComponent, SpriteComponent>([&](const TransformComponent& transform, const SpriteComponent& sprite)
		{
			SpriteData sprite_data;
			sprite_data.GPU_texture_view_handle = m_dx12_core.GetTextureManager()->ConvertTextureViewHandleToGPUTextureViewHandle(sprite.texture_handle);
			sprite_data.uv = sprite.uv;

			if (render_object_amount < m_transform_data_vector.size())
			{
				m_transform_data_vector[render_object_amount] = transform;
				m_sprite_data_vector[render_object_amount] = sprite_data;
			}
			else
			{
				m_transform_data_vector.push_back(transform);
				m_sprite_data_vector.push_back(sprite_data);
			}

			++render_object_amount;
		});
	Scene* draw_global_scene = SceneManager::GetSceneManager()->GetScene(GlobalScene::Get()->GetSceneIndex());
	draw_global_scene->GetEntityManager()->System<TransformComponent, SpriteComponent>([&](const TransformComponent& transform, const SpriteComponent& sprite)
		{
			SpriteData sprite_data;
			sprite_data.GPU_texture_view_handle = m_dx12_core.GetTextureManager()->ConvertTextureViewHandleToGPUTextureViewHandle(sprite.texture_handle);
			sprite_data.uv = sprite.uv;

			if (render_object_amount < m_transform_data_vector.size())
			{
				m_transform_data_vector[render_object_amount] = transform;
				m_sprite_data_vector[render_object_amount] = sprite_data;
			}
			else
			{
				m_transform_data_vector.push_back(transform);
				m_sprite_data_vector.push_back(sprite_data);
			}

			++render_object_amount;
		});

	//std::sort(draw_entities.begin(), draw_entities.end(), [&](uint32_t i, uint32_t j){
	//	return (m_transform_data_vector[i].GetPosition().z < m_transform_data_vector[j].GetPosition().z);
	//	});
	//std::cout << "Time: " << timer.StopTimer()/(double)Timer::TimeTypes::Milliseconds << " ms\n";


	DX12BufferHandle transform_data_buffer = 0;
	DX12BufferHandle sprite_data_buffer = 0;

	if (m_transform_data_vector.size())
	{
		transform_data_buffer = m_dx12_core.GetBufferManager()->AddBuffer(&m_dx12_core, sizeof(TransformComponent), render_object_amount, BufferType::CONSTANT_BUFFER);
		DX12BufferViewHandle transform_data_buffer_view = m_dx12_core.GetBufferManager()->AddView(&m_dx12_core, transform_data_buffer, ViewType::SHADER_RESOURCE_VIEW);
		m_dx12_core.GetCommandList()->SetConstantBuffer(&m_dx12_core, transform_data_buffer_view, 2);
		m_dx12_core.GetBufferManager()->UploadData(&m_dx12_core, transform_data_buffer, m_transform_data_vector.data(), sizeof(TransformComponent), render_object_amount);

		sprite_data_buffer = m_dx12_core.GetBufferManager()->AddBuffer(&m_dx12_core, sizeof(SpriteData), render_object_amount, BufferType::CONSTANT_BUFFER);
		DX12BufferViewHandle sprite_data_buffer_view = m_dx12_core.GetBufferManager()->AddView(&m_dx12_core, sprite_data_buffer, ViewType::SHADER_RESOURCE_VIEW);
		m_dx12_core.GetCommandList()->SetConstantBuffer(&m_dx12_core, sprite_data_buffer_view, 1);
		m_dx12_core.GetBufferManager()->UploadData(&m_dx12_core, sprite_data_buffer, m_sprite_data_vector.data(), sizeof(SpriteData), render_object_amount);
	}

	m_dx12_core.GetCommandList()->SetConstantBuffer(&m_dx12_core, m_quad_view_handle, 0);

	//Camera
	m_dx12_core.GetCommandList()->SetConstantBuffer(&m_dx12_core, m_camera_buffer_view, 3);

	CameraComponent active_camera = {};
	float view_size = 0.0f;
	draw_scene->GetEntityManager()->System<CameraComponent, TransformComponent>([&](CameraComponent& camera, const TransformComponent& transform)
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

		});
	draw_global_scene->GetEntityManager()->System<CameraComponent, TransformComponent>([&](CameraComponent& camera, const TransformComponent& transform)
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

		});
	m_dx12_core.GetBufferManager()->UploadData(&m_dx12_core, m_camera_buffer, &active_camera, sizeof(CameraComponent), 1);

	m_dx12_core.GetCommandList()->Draw(6, render_object_amount, 0, 0);

#ifdef _EDITOR
	m_dx12_core.GetCommandList()->SetRootSignature(&m_grid_root_signature);
	m_dx12_core.GetCommandList()->SetPipeline(&m_grid_pipeline);
	m_dx12_core.GetCommandList()->SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);

	m_dx12_core.GetCommandList()->SetConstantBuffer(&m_dx12_core, m_editor_lines_view_handle, 0);

	//Camera
	m_dx12_core.GetCommandList()->SetConstantBuffer(&m_dx12_core, m_camera_buffer_view, 1);

	m_dx12_core.GetCommandList()->SetConstantBuffer(&m_dx12_core, m_line_color_buffer_view, 2);
	Vector4 editor_line_color(1.0f, 1.0f, 1.0f, 1.0f);
	m_dx12_core.GetBufferManager()->UploadData(&m_dx12_core, m_line_color_buffer, &editor_line_color, sizeof(Vector4), 1);

	m_dx12_core.GetCommandList()->Draw(m_editor_lines_amount, 1, 0, 0);

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
#endif

	if (render_object_amount)
	{
		m_dx12_core.GetResourceDestroyer()->FreeBuffer(&m_dx12_core, transform_data_buffer);
		m_dx12_core.GetResourceDestroyer()->FreeBuffer(&m_dx12_core, sprite_data_buffer);
	}

	ImGUIMain::EndFrame(&m_dx12_core);

	m_dx12_core.GetCommandList()->TransitionTextureResource(&m_dx12_core, render_target_texture, ResourceState::PRESENT, ResourceState::RENDER_TARGET);

	m_dx12_core.GetCommandList()->Execute(&m_dx12_core, m_dx12_core.GetGraphicsCommandQueue());
	m_dx12_core.GetSwapChain()->Present();
	m_dx12_core.GetCommandList()->Signal(&m_dx12_core, m_dx12_core.GetGraphicsCommandQueue());

	m_dx12_core.EndOfFrame();

	m_debug_lines.clear();

	return m_window->WinMsg();
}

TextureHandle RenderCore::LoadTexture(const std::string& texture_file_name)
{
	AssetHandle texture_asset_handle = AssetManager::Get()->LoadTextureAsset(texture_file_name);

	if (m_texture_handles.contains(texture_asset_handle))
	{
		return m_texture_handles.find(texture_asset_handle)->second.texture_view_handle;
	}

	TextureInfo* texture_info = AssetManager::Get()->GetTextureData(texture_asset_handle);

	DX12TextureHandle texture_handle = m_dx12_core.GetTextureManager()->AddTexture(&m_dx12_core, texture_info, TextureFlags::NONE_FLAG);
	DX12TextureViewHandle texture_view_handle = m_dx12_core.GetTextureManager()->AddView(&m_dx12_core, texture_handle, ViewType::SHADER_RESOURCE_VIEW);

	m_texture_handles.insert({ texture_asset_handle, {texture_handle, texture_view_handle} });
	m_texture_to_asset.insert({ texture_view_handle, texture_asset_handle });

	return texture_view_handle;
}

AssetHandle RenderCore::GetTextureAssetHandle(TextureHandle texture_handle)
{
	if (m_texture_to_asset.contains(texture_handle))
	{
		return m_texture_to_asset.find(texture_handle)->second;
	}

	assert(false);
	return 0;
}

void RenderCore::AddLine(const Vector2& line)
{
	VertexGrid vertex_line;
	vertex_line.position[0] = line.x;
	vertex_line.position[1] = line.y;
	vertex_line.position[2] = 0.1f;
	m_debug_lines.push_back(vertex_line);
}

RenderCore* RenderCore::Get()
{
	return s_render_core;
}

Window* RenderCore::GetWindow()
{
	return m_window.get();
}
