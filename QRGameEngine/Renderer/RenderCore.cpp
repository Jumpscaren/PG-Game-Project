#include "pch.h"
#include "RenderCore.h"
#include "DX12CORE/DX12RootSignature.h"
#include "DX12CORE/DX12Pipeline.h"
#include "DX12CORE/DX12StackAllocator.h"
#include "SceneSystem/SceneManager.h"
#include "ECS/EntityManager.h"
#include "ImGUIMain.h"
#include "Asset/AssetManager.h"


RenderCore* RenderCore::s_render_core = nullptr;

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
		{{-1.0f, -1.0f, 0.0f}, {0.0f, 1.0f}, 0.0f},
		{{-1.0f, 1.0f, 0.0f}, {0.0f, 0.0f}, 0.0f},
		{{1.0f, 1.0f, 0.0f}, {1.0f, 0.0f}, 0.0f},

		{{-1.0f, -1.0f, 0.0f}, {0.0f, 1.0f}, 0.0f},
		{{1.0f, 1.0f, 0.0f}, {1.0f, 0.0f}, 0.0f},
		{{1.0f, -1.0f, 0.0f}, {1.0f, 1.0f}, 0.0f},
	};

	DX12BufferHandle quad_handle = m_dx12_core.GetBufferManager()->AddBuffer(&m_dx12_core, &quad, sizeof(Vertex), 6, BufferType::CONSTANT_BUFFER);
	m_quad_view_handle = m_dx12_core.GetBufferManager()->AddView(&m_dx12_core, quad_handle, ViewType::SHADER_RESOURCE_VIEW);

	m_stack_allocator = new DX12StackAllocator(&m_dx12_core, 1'000);

	m_root_signature
		.AddStaticSampler(&m_dx12_core, SamplerTypes::LINEAR_WRAP, 0)
		.AddConstant(&m_dx12_core, ShaderVisibility::VERTEX, 0)
		.AddConstant(&m_dx12_core, ShaderVisibility::PIXEL, 0)
		.AddConstant(&m_dx12_core, ShaderVisibility::VERTEX, 1)
		.InitRootSignature(&m_dx12_core);



	m_pipeline.InitPipeline(&m_dx12_core, &m_root_signature, L"../QRGameEngine/Shaders/VertexShader.hlsl", L"../QRGameEngine/Shaders/PixelShader.hlsl");

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

	m_dx12_core.GetCommandList()->TransitionTextureResource(&m_dx12_core, render_target_texture, ResourceState::RENDER_TARGET, ResourceState::PRESENT);

	m_dx12_core.GetCommandList()->ClearRenderTargetView(&m_dx12_core, m_dx12_core.GetSwapChain()->GetBackbufferView());
	m_dx12_core.GetCommandList()->ClearDepthStencilView(&m_dx12_core, m_depthstencil_view);

	m_dx12_core.GetCommandList()->SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_dx12_core.GetCommandList()->SetOMRenderTargets(&m_dx12_core, m_dx12_core.GetSwapChain()->GetBackbufferView(), m_depthstencil_view);

	m_dx12_core.GetCommandList()->SetViewport((uint64_t)m_window->GetWindowWidth(), (uint64_t)m_window->GetWindowHeight());
	m_dx12_core.GetCommandList()->SetScissorRect((uint64_t)m_window->GetWindowWidth(), (uint64_t)m_window->GetWindowHeight());

	uint64_t render_object_amount = 0;
	draw_scene->GetEntityManager()->System<TransformComponent, SpriteComponent>([&](TransformComponent& transform, SpriteComponent& sprite)
		{
			SpriteData sprite_data;
			sprite_data.GPU_texture_view_handle = m_dx12_core.GetTextureManager()->ConvertTextureViewHandleToGPUTextureViewHandle(sprite.texture_handle);
			sprite_data.uv.x = sprite.uv.x;
			sprite_data.uv.y = sprite.uv.y;

			if (render_object_amount < m_transform_data_vector.size())
			{
				m_transform_data_vector[render_object_amount] = std::move(transform);
				m_sprite_data_vector[render_object_amount] = sprite_data;
			}
			else
			{
				m_transform_data_vector.push_back(transform);
				m_sprite_data_vector.push_back(sprite_data);
			}

			++render_object_amount;
		});

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

	m_dx12_core.GetCommandList()->Draw(6, render_object_amount, 0, 0);

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

	return m_window->WinMsg();
}

TextureHandle RenderCore::CreateTexture(const std::string& texture_file_name)
{
	AssetHandle texture_asset_handle = AssetManager::Get()->LoadTexture(texture_file_name);

	if (m_texture_handles.contains(texture_asset_handle))
	{
		return m_texture_handles.find(texture_asset_handle)->second.texture_view_handle;
	}

	TextureInfo* texture_info = AssetManager::Get()->GetTextureData(texture_asset_handle);

	DX12TextureHandle texture_handle = m_dx12_core.GetTextureManager()->AddTexture(&m_dx12_core, texture_info, TextureFlags::NONE_FLAG);
	DX12TextureViewHandle texture_view_handle = m_dx12_core.GetTextureManager()->AddView(&m_dx12_core, texture_handle, ViewType::SHADER_RESOURCE_VIEW);

	m_texture_handles.insert({texture_asset_handle, {texture_handle, texture_view_handle}});

	return texture_view_handle;
}

RenderCore* RenderCore::Get()
{
	return s_render_core;
}
