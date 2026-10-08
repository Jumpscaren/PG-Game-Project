#pragma once
#include "pch.h"
#include "ViewTypes.h"
#include "DescriptorTypes.h"
#include "DX12MemAllocInclude.h"

enum class BufferType
{
	CONSTANT_BUFFER,
	MODIFIABLE_BUFFER,
};

struct DX12Buffer
{
	Microsoft::WRL::ComPtr<ID3D12Resource> buffer_resource;
	Microsoft::WRL::ComPtr<D3D12MA::Allocation> buffer_allocation;
	uint64_t nr_of_elements;
	uint64_t element_size;
	uint64_t aligned_buffer_size;
	//For frames in flight
	BufferType buffer_type;
	uint64_t real_nr_of_elements;
};

using DX12BufferHandle = uint64_t;

struct DX12BufferView
{
	DX12BufferHandle buffer_handle;
	ViewType buffer_view_type;
	std::vector<DescriptorHandle> buffer_descriptor_handles;
};

struct DX12BufferSubAllocation
{
	DX12BufferHandle handle;
	uint64_t offset = 0;
	uint64_t size = 0;
	uint64_t real_size = 0;
	BufferType buffer_type;
};