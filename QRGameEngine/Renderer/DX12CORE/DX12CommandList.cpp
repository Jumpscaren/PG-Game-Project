#include "pch.h"
#include "DX12CommandList.h"
#include "DX12Core.h"
#include "DX12Fence.h"
#include "DX12RootSignature.h"
#include "DX12Pipeline.h"

ID3D12GraphicsCommandList4* DX12CommandList::GetCommandList()
{
	return m_command_list.Get();
}

DX12CommandList::~DX12CommandList()
{

}

void DX12CommandList::InitCommandList(DX12Core* dx12_core, CommandListType command_type)
{
	HRESULT hr = dx12_core->GetDevice()->CreateCommandAllocator((D3D12_COMMAND_LIST_TYPE)command_type, IID_PPV_ARGS(m_command_allocator.GetAddressOf()));
	assert(SUCCEEDED(hr));

	ID3D12CommandList *temp_list;
	hr = dx12_core->GetDevice()->CreateCommandList(0, (D3D12_COMMAND_LIST_TYPE)command_type, m_command_allocator.Get(),
		nullptr, IID_PPV_ARGS(&temp_list));
	assert(SUCCEEDED(hr));

	hr = temp_list->QueryInterface(__uuidof(ID3D12GraphicsCommandList4),
		reinterpret_cast<void**>(m_command_list.GetAddressOf()));
	assert(SUCCEEDED(hr));

	temp_list->Release();

	m_fence = std::make_unique<DX12Fence>();
	m_fence->InitFence(dx12_core);
}

void DX12CommandList::Execute(DX12Core* dx12_core, DX12CommandQueue* command_queue)
{
	HRESULT hr = m_command_list->Close();
	assert(SUCCEEDED(hr));
	command_queue->ExecuteCommandList(dx12_core, this);

	reset_command_list = true;
}

void DX12CommandList::Reset()
{
	if (!reset_command_list)
	{
		std::cout << "Command list is reseted before execute of the command list!\n";
		return;
	}

	reset_command_list = false;

	HRESULT hr = m_command_allocator->Reset();
	assert(SUCCEEDED(hr));
	hr = m_command_list->Reset(m_command_allocator.Get(), nullptr);
	assert(SUCCEEDED(hr));
}

void DX12CommandList::SetShaderBindableDescriptorHeap(DX12Core* dx12_core)
{
	auto tmp = dx12_core->GetDescriptorManager()->GetShaderBindable()->GetDescriptorHeap();
	m_command_list->SetDescriptorHeaps(1, &tmp);
}

void DX12CommandList::SetRootSignature(DX12RootSignature* root_signature)
{
	m_command_list->SetGraphicsRootSignature(root_signature->GetRootSignature());
}

void DX12CommandList::SetPipeline(DX12Pipeline* pipeline)
{
	m_command_list->SetPipelineState(pipeline->GetPipeline());
}

void DX12CommandList::SignalAndWait(DX12Core* dx12_core, DX12CommandQueue* command_queue)
{
	m_fence->SignalAndWait(dx12_core, command_queue);
}

void DX12CommandList::Signal(DX12Core* dx12_core, DX12CommandQueue* command_queue)
{
	m_fence->Signal(dx12_core, command_queue);
}

void DX12CommandList::Wait(DX12Core* dx12_core)
{
	m_fence->Wait(dx12_core);
}

void DX12CommandList::Throttle(DX12Core* dx12_core)
{
	Execute(dx12_core, dx12_core->GetGraphicsCommandQueue());
	Signal(dx12_core, dx12_core->GetGraphicsCommandQueue());
	//Wait(dx12_core);
	//Reset();
}

void DX12CommandList::CopyBufferRegion(ID3D12Resource* destination_buffer, ID3D12Resource* source_buffer, uint64_t destination_offset, uint64_t source_offset, uint64_t size)
{
	m_command_list->CopyBufferRegion(destination_buffer, destination_offset, source_buffer, source_offset, size);
}

void DX12CommandList::TransitionResource(ID3D12Resource* transition_resource, const ResourceState& new_state, const ResourceState& old_state)
{
	D3D12_RESOURCE_BARRIER barrier;
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = transition_resource;
	barrier.Transition.StateBefore = (D3D12_RESOURCE_STATES)old_state;
	barrier.Transition.StateAfter = (D3D12_RESOURCE_STATES)new_state;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	m_command_list->ResourceBarrier(1, &barrier);
}

void DX12CommandList::TransitionTextureResource(DX12Core* dx12_core, DX12TextureHandle texture_handle, const ResourceState& new_state, const ResourceState& old_state)
{
	TransitionResource(dx12_core->GetTextureManager()->GetTextureResource(texture_handle), new_state, old_state);
}

void DX12CommandList::TransitionBufferResource(DX12Core* dx12_core, DX12BufferHandle buffer_handle, const ResourceState& new_state, const ResourceState& old_state)
{
	TransitionResource(dx12_core->GetBufferManager()->GetBufferResource(buffer_handle), new_state, old_state);
}

void DX12CommandList::ClearRenderTargetView(DX12Core* dx12_core, DX12TextureViewHandle texture_view_handle)
{
	float clearColour[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	m_command_list->ClearRenderTargetView(dx12_core->GetTextureManager()->GetTextureView(texture_view_handle)->texture_descriptor_handle.cpu_handle, clearColour, 0, nullptr);
}

void DX12CommandList::ClearDepthStencilView(DX12Core* dx12_core, DX12TextureViewHandle texture_view_handle)
{
	m_command_list->ClearDepthStencilView(dx12_core->GetTextureManager()->GetTextureView(texture_view_handle)->texture_descriptor_handle.cpu_handle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
}

void DX12CommandList::SetPrimitiveTopology(const D3D_PRIMITIVE_TOPOLOGY& topology)
{
	m_command_list->IASetPrimitiveTopology(topology);
}

void DX12CommandList::SetOMRenderTargets(DX12Core* dx12_core, DX12TextureViewHandle render_target_view, DX12TextureViewHandle depthstencil_view)
{
	m_command_list->OMSetRenderTargets(1, &dx12_core->GetTextureManager()->GetTextureView(render_target_view)->texture_descriptor_handle.cpu_handle,
		true, &dx12_core->GetTextureManager()->GetTextureView(depthstencil_view)->texture_descriptor_handle.cpu_handle);
}

void DX12CommandList::SetViewport(uint64_t width, uint64_t height)
{
	D3D12_VIEWPORT viewport = { 0, 0, static_cast<float>(width), static_cast<float>(height), 0.0f, 1.0f };
	m_command_list->RSSetViewports(1, &viewport);
}

void DX12CommandList::SetScissorRect(uint64_t width, uint64_t height)
{
	D3D12_RECT scissorRect = { 0, 0, static_cast<long>(width), static_cast<long>(height) };
	m_command_list->RSSetScissorRects(1, &scissorRect);
}

void DX12CommandList::SetBufferDescriptorTable(DX12Core* dx12_core, DX12BufferViewHandle buffer_view_handle, uint64_t root_parameter_index)
{
	auto descriptor = dx12_core->GetBufferManager()->GetDescriptorHandle(dx12_core, buffer_view_handle);

	m_command_list->SetGraphicsRootDescriptorTable((UINT)root_parameter_index, descriptor->gpu_handle);
}

void DX12CommandList::SetTextureDescriptorTable(DX12Core* dx12_core, DX12TextureViewHandle texture_view_handle, uint64_t root_parameter_index)
{
	m_command_list->SetGraphicsRootDescriptorTable((UINT)root_parameter_index, dx12_core->GetTextureManager()->GetTextureView(texture_view_handle)->texture_descriptor_handle.gpu_handle);
}

void DX12CommandList::SetConstant(DX12Core* dx12_core, uint32_t constant, uint64_t root_parameter_index)
{
	m_command_list->SetGraphicsRoot32BitConstant((UINT)root_parameter_index, constant, 0);
}

void DX12CommandList::SetConstantTexture(DX12Core* dx12_core, DX12TextureViewHandle texture_view_handle, uint64_t root_parameter_index)
{
	m_command_list->SetGraphicsRoot32BitConstant((UINT)root_parameter_index, dx12_core->GetTextureManager()->GetTextureView(texture_view_handle)->texture_descriptor_handle.descriptor_offset, 0);
}

void DX12CommandList::SetConstantBuffer(DX12Core* dx12_core, DX12BufferViewHandle buffer_view_handle, uint64_t root_parameter_index)
{
	auto descriptor = dx12_core->GetBufferManager()->GetDescriptorHandle(dx12_core, buffer_view_handle);

	m_command_list->SetGraphicsRoot32BitConstant((UINT)root_parameter_index, descriptor->descriptor_offset, 0);
}

void DX12CommandList::Draw(uint64_t vertices, uint64_t nr_of_objects, uint64_t start_vertex, uint64_t start_object)
{
	m_command_list->DrawInstanced((UINT)vertices, (UINT)nr_of_objects, (UINT)start_vertex, (UINT)start_object);
}

void DX12CommandList::CopyTextureRegion(D3D12_TEXTURE_COPY_LOCATION* destination, D3D12_TEXTURE_COPY_LOCATION* source)
{
	m_command_list->CopyTextureRegion(destination, 0, 0, 0, source, nullptr);
}

void DX12CommandList::CopyTexture(DX12Core* dx12_core, DX12TextureHandle destination, DX12TextureHandle source)
{
	ID3D12Resource* texture_destination = dx12_core->GetTextureManager()->GetTextureResource(destination);
	ID3D12Resource* texture_source = dx12_core->GetTextureManager()->GetTextureResource(source);
	m_command_list->CopyResource(texture_destination, texture_source);
}
