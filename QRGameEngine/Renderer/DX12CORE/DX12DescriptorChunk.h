#pragma once
#include "HelpTypes.h"
#include "DescriptorTypes.h"

class DX12Core;

struct DescriptorRange
{
	uint32_t start_of_range;
	uint32_t end_of_range;
};

class DX12DescriptorChunk
{
private:
	DescriptorHeapTypes m_descriptor_type;
	D3D12_CPU_DESCRIPTOR_HANDLE m_cpu_handle;
	D3D12_GPU_DESCRIPTOR_HANDLE m_gpu_handle;
	uint32_t m_descriptor_size;
	uint32_t m_number_descriptors;
	uint32_t m_base_offest;
	std::vector<DescriptorRange> m_ranges;

private:
	void AddDescriptorOffset(uint32_t descriptor_offset);
	uint32_t GetDescriptorOffset();

public:
	DX12DescriptorChunk();
	DX12DescriptorChunk(DX12Core* dx12_core, DescriptorHeapTypes descriptor_type, D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle, D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle, uint32_t number_descriptors, uint32_t base_offset);

	DescriptorHandle AddDescriptor();
	void RemoveDescriptor(DescriptorHandle& descriptor_handle);
};

