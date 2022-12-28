#pragma once
#include "HelpTypes.h"
#include "DX12DescriptorChunk.h"

class DX12Core;

class DX12DescriptorHeap
{
private:
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_descriptor_heap;
	uint32_t m_decriptor_heap_current_offset = 0;
	uint32_t m_max_descriptor_heap_offset;
	bool m_shader_visible;
	DescriptorHeapTypes m_descriptor_heap_type;

public:
	DX12DescriptorHeap() = default;
	void InitDescriptorHeap(DX12Core* dx12_core, DescriptorHeapTypes descriptor_heap_type, uint32_t max_views, bool shader_visible);
	DX12DescriptorChunk GetDescriptorChunk(DX12Core* dx12_core, const DescriptorHeapTypes& descriptor_type, uint32_t descriptors);
	ID3D12DescriptorHeap* GetDescriptorHeap();
};

