#include "pch.h"
#include "DX12DescriptorChunk.h"
#include "DX12Core.h"

void DX12DescriptorChunk::AddDescriptorOffset(uint32_t descriptor_offset)
{
	assert(descriptor_offset <= m_base_offest + m_descriptor_size);

	if (m_ranges.size() == 0)
	{
		DescriptorRange range = {};
		range.start_of_range = descriptor_offset;
		range.end_of_range = descriptor_offset;
		m_ranges.push_back(range);
		return;
	}

	for (uint32_t i = 0; i < m_ranges.size(); ++i)
	{
		assert(!(m_ranges[i].start_of_range <= descriptor_offset && m_ranges[i].end_of_range >= descriptor_offset) && "Invalid range, the range already exist");

		//If range start is 2 and the offset is 1 then we want to add it to the range by -- the start range
		if (m_ranges[i].start_of_range - 1 == descriptor_offset)
		{
			--m_ranges[i].start_of_range;
			return;
		}
		//If range end is 2 and the offset is 3 then we want to add it to the range by ++ the end range
		else if (descriptor_offset - 1 == m_ranges[i].end_of_range)
		{
			++m_ranges[i].end_of_range;

			//If there exist a next range, then if range start is 2 and the offset 1 then we want to merge the two ranges 
			if (m_ranges.size() - 1 != i && descriptor_offset + 1 == m_ranges[i + 1].start_of_range)
			{
				m_ranges[i].end_of_range = m_ranges[i + 1].end_of_range;
				m_ranges.erase(m_ranges.begin() + i + 1);
				//Might be real bad!
				if (m_ranges.size() < m_ranges.capacity() / 2)
					m_ranges.shrink_to_fit();
			}
			return;
		}
		else if (descriptor_offset < m_ranges[i].start_of_range)
		{
			DescriptorRange range = {};
			range.start_of_range = descriptor_offset;
			range.end_of_range = descriptor_offset;
			m_ranges.insert(m_ranges.begin() + i, 1, range);
			return;
		}
		else if (descriptor_offset > m_ranges[i].end_of_range)
		{
			uint32_t insert_index = i + 1;
			//Check if there exist a range after the current range
			if (m_ranges.size() - 1 != i)
			{
				//Check if the next range start range is smaller then the offset, then we check the next range
				if (descriptor_offset > m_ranges[i + 1].start_of_range)
					continue;
			}
			DescriptorRange range = {};
			range.start_of_range = descriptor_offset;
			range.end_of_range = descriptor_offset;
			m_ranges.insert(m_ranges.begin() + insert_index, 1, range);
			return;
		}
	}
}

uint32_t DX12DescriptorChunk::GetDescriptorOffset()
{
	assert(m_ranges.size());

	uint32_t descriptor_offset = m_ranges[0].start_of_range++;

	if (m_ranges[0].start_of_range > m_ranges[0].end_of_range)
		m_ranges.erase(m_ranges.begin());

	return descriptor_offset;
}

DX12DescriptorChunk::DX12DescriptorChunk()
{
	m_descriptor_type = DescriptorHeapTypes::RENDERTARGET_VIEW;
	m_cpu_handle = D3D12_CPU_DESCRIPTOR_HANDLE(0);
	m_gpu_handle = D3D12_GPU_DESCRIPTOR_HANDLE(0);
	m_number_descriptors = 0; 
	m_base_offest = 0;
	m_descriptor_size = 0;
}

DX12DescriptorChunk::DX12DescriptorChunk(DX12Core* dx12_core, DescriptorHeapTypes descriptor_type, D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle, D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle, uint32_t number_descriptors, uint32_t base_offset)
	: m_descriptor_type(descriptor_type), m_cpu_handle(cpu_handle), m_gpu_handle(gpu_handle), m_number_descriptors(number_descriptors), m_base_offest(base_offset)
{
	switch (descriptor_type)
	{
	case DescriptorHeapTypes::RENDERTARGET_VIEW:
		m_descriptor_size = dx12_core->GetDevice()->GetDescriptorHandleIncrementSize((D3D12_DESCRIPTOR_HEAP_TYPE)DescriptorHeapTypes::RENDERTARGET_VIEW);
		break;
	case DescriptorHeapTypes::DEPTHSTENCIL_VIEW:
		m_descriptor_size = dx12_core->GetDevice()->GetDescriptorHandleIncrementSize((D3D12_DESCRIPTOR_HEAP_TYPE)DescriptorHeapTypes::DEPTHSTENCIL_VIEW);;
		break;
	case DescriptorHeapTypes::SHADERBINDABLE_VIEW:
		m_descriptor_size = dx12_core->GetDevice()->GetDescriptorHandleIncrementSize((D3D12_DESCRIPTOR_HEAP_TYPE)DescriptorHeapTypes::SHADERBINDABLE_VIEW);;
		break;
	default:
		m_descriptor_size = 0;
		assert(false);
	}

	DescriptorRange base_range = {};
	base_range.start_of_range = m_base_offest;
	base_range.end_of_range = m_base_offest + m_number_descriptors - 1;
	m_ranges.push_back(base_range);
}

DescriptorHandle DX12DescriptorChunk::AddDescriptor()
{
	assert(m_ranges.size());

	uint32_t descriptor_offset = GetDescriptorOffset();

	D3D12_CPU_DESCRIPTOR_HANDLE heap_handle = m_cpu_handle;
	heap_handle.ptr += m_descriptor_size * descriptor_offset;

	D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle = m_gpu_handle;
	gpu_handle.ptr += m_descriptor_size * descriptor_offset;

	DescriptorHandle descriptor_handle = {};
	descriptor_handle.descriptor_offset = descriptor_offset;
	descriptor_handle.cpu_handle = heap_handle;
	descriptor_handle.gpu_handle = gpu_handle;

	return std::move(descriptor_handle);
}

void DX12DescriptorChunk::RemoveDescriptor(DescriptorHandle& descriptor_handle)
{
	AddDescriptorOffset(descriptor_handle.descriptor_offset);
	descriptor_handle.descriptor_offset = -1;
}
