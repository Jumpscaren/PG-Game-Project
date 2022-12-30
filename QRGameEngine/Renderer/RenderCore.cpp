#include "pch.h"
#include "RenderCore.h"
#include "DX12CORE/DX12RootSignature.h"
#include "DX12CORE/DX12Pipeline.h"
#include "DX12CORE/DX12StackAllocator.h"

void RenderCore::AddRenderObject(DX12TextureViewHandle texture_view_handle, const DirectX::XMMATRIX& world_matrix)
{
	m_world_matrices.push_back(world_matrix);

	RenderObject render_object = {};
	render_object.texture_view_handle = texture_view_handle;
	render_object.world_matrix_index = m_world_matrices.size() - 1;

	m_render_objects.push_back(render_object);
}

DX12BufferViewHandle transform_constant_buffer_view;
DX12BufferHandle transform_sub;
RenderCore::RenderCore(uint32_t window_width, uint32_t window_height, const std::wstring& window_name)
{
	m_window = std::make_unique<Window>(window_width, window_height, window_name, window_name);
	m_dx12_core.InitCore(m_window.get(), 2);

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

	TextureInfo texture_info = m_dx12_core.GetTextureManager()->LoadTextureFromFile("../QRGameEngine/Textures/Temp.png");
	DX12TextureHandle image_handle = m_dx12_core.GetTextureManager()->AddTexture(&m_dx12_core, texture_info, TextureFlags::NONE_FLAG);
	DX12TextureViewHandle texture_view_handle = m_dx12_core.GetTextureManager()->AddView(&m_dx12_core, image_handle, ViewType::SHADER_RESOURCE_VIEW);

	m_stack_allocator = new DX12StackAllocator(&m_dx12_core, 1'000);

	DirectX::XMMATRIX matrix = DirectX::XMMatrixScaling(0.2f, 0.2f, 0.2f) * DirectX::XMMatrixRotationRollPitchYaw(0.0f, 0.0f, DirectX::XM_PIDIV4 / 2.0f) * DirectX::XMMatrixTranslation(-0.5f, -0.5f, 0.0f);
	DirectX::XMMATRIX scaled_matrix = DirectX::XMMatrixScaling(0.2f, 0.2f, 0.2f) * DirectX::XMMatrixRotationRollPitchYaw(0.0f, 0.0f, -DirectX::XM_PIDIV4 / 2.0f) * DirectX::XMMatrixTranslation(0.5f, 0.5f, 0.0f);
	DirectX::XMMATRIX m_3= DirectX::XMMatrixScaling(0.2f, 0.2f, 0.2f) * DirectX::XMMatrixRotationRollPitchYaw(0.0f, 0.0f, -DirectX::XM_PIDIV4 / 2.0f) * DirectX::XMMatrixTranslation(-0.1f, 0.0f, 0.0f);

	AddRenderObject(texture_view_handle, matrix);
	AddRenderObject(texture_view_handle, scaled_matrix);
	AddRenderObject(texture_view_handle, m_3);

	//for (int i = 0; i < 100'000; ++i)
	//{
	//	matrix = DirectX::XMMatrixScaling(0.2f, 0.2f, 0.2f) * DirectX::XMMatrixRotationRollPitchYaw(0.0f, 0.0f, i) * DirectX::XMMatrixTranslation(-0.5f + (float)i / 100'000.0f, -0.5f + (float)i / 100'000.0f, 0.0f);
	//	AddRenderObject(texture_view_handle, matrix);
	//}

	transform_sub = m_dx12_core.GetBufferManager()->AddBuffer(&m_dx12_core, sizeof(DirectX::XMMATRIX), m_world_matrices.size(), BufferType::MODIFIABLE_BUFFER);
	transform_constant_buffer_view = m_dx12_core.GetBufferManager()->AddView(&m_dx12_core, transform_sub, ViewType::SHADER_RESOURCE_VIEW);

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

	delete[] texture_info.texture_data;
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

	for (auto& render_object : m_render_objects)
	{
		m_world_matrices[render_object.world_matrix_index].r[3].m128_f32[0] += 0.0001f;
	}

	m_dx12_core.GetCommandList()->SetConstantBuffer(&m_dx12_core, m_quad_view_handle, 0);

	DX12BufferHandle transforms = m_dx12_core.GetBufferManager()->AddBuffer(&m_dx12_core, sizeof(DirectX::XMMATRIX), m_world_matrices.size(), BufferType::CONSTANT_BUFFER);
	DX12BufferViewHandle transforms_view = m_dx12_core.GetBufferManager()->AddView(&m_dx12_core, transforms, ViewType::SHADER_RESOURCE_VIEW);
	m_dx12_core.GetCommandList()->SetConstantBuffer(&m_dx12_core, transforms_view, 2);
	m_dx12_core.GetBufferManager()->UploadData(&m_dx12_core, transforms, m_world_matrices.data(), sizeof(DirectX::XMMATRIX), m_world_matrices.size());

	//m_dx12_core.GetBufferManager()->UploadData(&m_dx12_core, transform_sub, m_world_matrices.data(), sizeof(DirectX::XMMATRIX), m_world_matrices.size());
	//m_dx12_core.GetCommandList()->SetConstantBuffer(&m_dx12_core, transform_constant_buffer_view, 2);

	m_dx12_core.GetCommandList()->SetConstantTexture(&m_dx12_core, m_render_objects[0].texture_view_handle, 1);
	m_dx12_core.GetCommandList()->Draw(6, m_render_objects.size(), 0, 0);

	//m_dx12_core.GetCommandList()->TransitionBufferResource(&m_dx12_core, transform_sub, ResourceState::COMMON, ResourceState::VERTEX_AND_CONSTANT_BUFFER);

	m_dx12_core.GetCommandList()->TransitionTextureResource(&m_dx12_core, render_target_texture, ResourceState::PRESENT, ResourceState::RENDER_TARGET);

	m_dx12_core.GetResourceDestroyer()->FreeBuffer(&m_dx12_core, transforms);

	m_dx12_core.GetCommandList()->Execute(&m_dx12_core, m_dx12_core.GetGraphicsCommandQueue());
	m_dx12_core.GetSwapChain()->Present();
	m_dx12_core.GetCommandList()->Signal(&m_dx12_core, m_dx12_core.GetGraphicsCommandQueue());

	m_dx12_core.EndOfFrame();

	return m_window->WinMsg();
}
