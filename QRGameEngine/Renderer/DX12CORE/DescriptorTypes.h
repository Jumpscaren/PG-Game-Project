#pragma once

struct DescriptorHandle
{
	uint32_t descriptor_offset;
	D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle;
	D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle;
};