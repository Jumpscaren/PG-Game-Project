#pragma once
#include "BufferTypes.h"

class DX12Core;

class DX12StackAllocator
{
private:
	DX12BufferHandle m_buffer_handle;

	uint64_t m_current_memory_offset;
	uint64_t m_allocator_size;
	uint64_t m_alignment_size;

private:
	uint64_t GetAlignedSize(uint64_t size);

public:
	DX12StackAllocator() = delete;
	DX12StackAllocator(DX12Core* dx12_core, uint64_t allocator_size, uint64_t alignment = 256);

	DX12BufferSubAllocation Allocate(uint64_t size, const BufferType& buffer_type);
	void Free();
};

