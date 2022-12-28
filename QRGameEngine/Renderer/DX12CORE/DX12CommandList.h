#pragma once
#include "HelpTypes.h"
#include "TextureTypes.h"
#include "BufferTypes.h"

class DX12Core;
class DX12Fence;
class DX12CommandQueue;
class DX12RootSignature;
class DX12Pipeline;

class DX12CommandList
{
	friend DX12CommandQueue;

private:
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_command_allocator;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList4> m_command_list;
	std::unique_ptr<DX12Fence> m_fence;
	bool reset_command_list = false;

private:
	ID3D12GraphicsCommandList4* GetCommandList();

public:
	DX12CommandList() = default;
	~DX12CommandList();
	void InitCommandList(DX12Core* dx12_core, CommandListType command_type);
	void Execute(DX12Core* dx12_core, DX12CommandQueue* command_queue);
	void Reset();
	void SetShaderBindableDescriptorHeap(DX12Core* dx12_core);
	void SetRootSignature(DX12RootSignature* root_signature);
	void SetPipeline(DX12Pipeline* pipeline);
	void SignalAndWait(DX12Core* dx12_core, DX12CommandQueue* command_queue);
	void Signal(DX12Core* dx12_core, DX12CommandQueue* command_queue);
	void Wait(DX12Core* dx12_core);
	void CopyBufferRegion(ID3D12Resource* destination_buffer, ID3D12Resource* source_buffer, uint64_t destination_offset, uint64_t source_offset, uint64_t size);
	void TransitionResource(ID3D12Resource* transition_resource, const ResourceState& new_state, const ResourceState& old_state);
	void TransitionTextureResource(DX12Core* dx12_core, DX12TextureHandle texture_handle, const ResourceState& new_state, const ResourceState& old_state);
	void TransitionBufferResource(DX12Core* dx12_core, DX12BufferHandle buffer_handle, const ResourceState& new_state, const ResourceState& old_state);
	void ClearRenderTargetView(DX12Core* dx12_core, DX12TextureViewHandle texture_view_handle);
	void ClearDepthStencilView(DX12Core* dx12_core, DX12TextureViewHandle texture_view_handle);
	void SetPrimitiveTopology(const D3D_PRIMITIVE_TOPOLOGY& topology);
	void SetOMRenderTargets(DX12Core* dx12_core, DX12TextureViewHandle render_target_view, DX12TextureViewHandle depthstencil_view);
	void SetViewport(uint64_t width, uint64_t height);
	void SetScissorRect(uint64_t width, uint64_t height);
	void SetBufferDescriptorTable(DX12Core* dx12_core, DX12BufferViewHandle buffer_view_handle, uint64_t root_parameter_index);
	void SetTextureDescriptorTable(DX12Core* dx12_core, DX12TextureViewHandle texture_view_handle, uint64_t root_parameter_index);
	void SetConstant(DX12Core* dx12_core, uint32_t constant, uint64_t root_parameter_index);
	void SetConstantTexture(DX12Core* dx12_core, DX12TextureViewHandle texture_view_handle, uint64_t root_parameter_index);
	void SetConstantBuffer(DX12Core* dx12_core, DX12BufferViewHandle buffer_view_handle, uint64_t root_parameter_index);
	void Draw(uint64_t vertices, uint64_t nr_of_objects, uint64_t start_vertex, uint64_t start_object);
	void CopyTextureRegion(D3D12_TEXTURE_COPY_LOCATION* destination, D3D12_TEXTURE_COPY_LOCATION* source);
};

