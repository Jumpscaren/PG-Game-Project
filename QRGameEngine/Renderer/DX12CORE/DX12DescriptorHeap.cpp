#include "pch.h"
#include "DX12DescriptorHeap.h"
#include "DX12Core.h"

void DX12DescriptorHeap::InitDescriptorHeap(DX12Core* dx12_core, DescriptorHeapTypes descriptor_heap_type, uint32_t max_views, bool shader_visible)
{
	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.Type = (D3D12_DESCRIPTOR_HEAP_TYPE)descriptor_heap_type;
	desc.NumDescriptors = max_views;
	desc.Flags = shader_visible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	desc.NodeMask = 0;

	HRESULT hr = dx12_core->GetDevice()->CreateDescriptorHeap(&desc, IID_PPV_ARGS(m_descriptor_heap.GetAddressOf()));
	assert(SUCCEEDED(hr));

	m_decriptor_heap_current_offset = 0;
	m_max_descriptor_heap_offset = max_views;
	m_descriptor_heap_type = descriptor_heap_type;
	m_shader_visible = shader_visible;
}

DX12DescriptorChunk DX12DescriptorHeap::GetDescriptorChunk(DX12Core* dx12_core, const DescriptorHeapTypes& descriptor_type, uint32_t descriptors)
{
	if (descriptors < m_decriptor_heap_current_offset)
		assert(false);

	auto cpu_handle = m_descriptor_heap->GetCPUDescriptorHandleForHeapStart();
	auto gpu_handle = m_descriptor_heap->GetGPUDescriptorHandleForHeapStart();

	uint32_t old_offset = m_decriptor_heap_current_offset;
	m_decriptor_heap_current_offset += descriptors;

	return DX12DescriptorChunk(dx12_core, descriptor_type, cpu_handle, gpu_handle, descriptors, old_offset);
}

ID3D12DescriptorHeap* DX12DescriptorHeap::GetDescriptorHeap()
{
	return m_descriptor_heap.Get();
}
