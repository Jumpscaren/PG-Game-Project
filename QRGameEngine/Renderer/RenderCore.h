#pragma once
#include "DX12CORE/DX12Core.h"
#include "DX12CORE/DX12RootSignature.h"
#include "DX12CORE/DX12Pipeline.h"
#include "Window.h"

class DX12StackAllocator;

struct RenderObject
{
	DX12TextureViewHandle texture_view_handle;
	uint64_t world_matrix_index;
	//DirectX::XMMATRIX world_matrix;
};

class RenderCore
{
private:
	DX12Core m_dx12_core;
	std::unique_ptr<Window> m_window;
	DX12TextureHandle m_depthstencil;
	DX12TextureViewHandle m_depthstencil_view;
	DX12RootSignature m_root_signature;
	DX12Pipeline m_pipeline;
	DX12BufferViewHandle m_quad_view_handle;
	DX12StackAllocator* m_stack_allocator;
	//DX12BufferSubAllocation m_transform_sub_buffer;
	//DX12BufferViewHandle m_transform_constant_buffer_view;

	std::vector<RenderObject> m_render_objects;
	std::vector<DirectX::XMMATRIX> m_world_matrices;

private:
	void AddRenderObject(DX12TextureViewHandle texture_view_handle, const DirectX::XMMATRIX& world_matrix);

public:
	RenderCore(uint32_t window_width, uint32_t window_height, const std::wstring& window_name);
	bool UpdateRender();
};

