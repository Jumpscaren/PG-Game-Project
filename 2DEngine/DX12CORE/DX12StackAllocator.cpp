#include "pch.h"
#include "DX12StackAllocator.h"
#include "DX12Core.h"

DX12StackAllocator::DX12StackAllocator(DX12Core* dx12_core, uint64_t allocator_size, uint64_t alignment) : m_alignment_size(alignment)
{
	m_allocator_size = GetAlignedSize(allocator_size);

	m_buffer_handle = dx12_core->GetBufferManager()->AddBuffer(dx12_core, m_allocator_size, 1, BufferType::CONSTANT_BUFFER);

	m_current_memory_offset = 0;
}

DX12BufferSubAllocation DX12StackAllocator::Allocate(uint64_t size, const BufferType& buffer_type)
{
	size = GetAlignedSize(size);

	uint64_t real_size = size;
	if (buffer_type == BufferType::MODIFIABLE_BUFFER)
	{
		real_size *= DX12Core::GetFramesInFlight();
	}

	assert(m_current_memory_offset + real_size <= m_allocator_size);

	DX12BufferSubAllocation sub_allocation;
	sub_allocation.handle = m_buffer_handle;
	sub_allocation.offset = m_current_memory_offset;
	sub_allocation.size = size;
	sub_allocation.real_size = real_size;
	sub_allocation.buffer_type = buffer_type;

	m_current_memory_offset += real_size;

	return sub_allocation;
}

void DX12StackAllocator::Free()
{
	assert(false);
	m_current_memory_offset = 0;
}

uint64_t DX12StackAllocator::GetAlignedSize(uint64_t size)
{
	return ((size + (m_alignment_size - 1)) & ~(m_alignment_size - 1));
}
