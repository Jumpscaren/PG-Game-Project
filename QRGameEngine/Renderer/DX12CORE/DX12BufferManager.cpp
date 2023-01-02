#include "pch.h"
#include "DX12BufferManager.h"
#include "DX12Core.h"
#include "../Helpers/HandleManager.h"

DX12Buffer* DX12BufferManager::GetDX12Buffer(DX12BufferHandle handle)
{
	return &m_buffers[handle];
}

DX12BufferHandle DX12BufferManager::AddBuffer(Microsoft::WRL::ComPtr<ID3D12Resource> buffer, Microsoft::WRL::ComPtr<D3D12MA::Allocation> buffer_allocation, 
	const ResourceState& state, uint64_t element_size, uint64_t nr_of_elements, const BufferType& buffer_type, uint64_t real_nr_of_elements, uint64_t aligned_buffer_size)
{
	DX12BufferHandle handle = 0;
	DX12Buffer new_buffer;
	new_buffer.buffer_resource = buffer;
	new_buffer.buffer_allocation = buffer_allocation;
	new_buffer.element_size = element_size;
	new_buffer.nr_of_elements = nr_of_elements;
	new_buffer.buffer_type = buffer_type;
	new_buffer.real_nr_of_elements;
	new_buffer.aligned_buffer_size = aligned_buffer_size;

	if (HandleManager::GetHandle(HandleManager::HandleType::BUFFER, handle))
	{
		m_buffer_to_views.insert({ handle, {} });

		m_buffers[handle] = new_buffer;
		return handle;
	}

	m_buffer_to_views.insert({ m_buffers.size(), {} });

	m_buffers.push_back(new_buffer);
	return m_buffers.size() - 1;
}

DX12BufferViewHandle DX12BufferManager::AddView(DX12BufferHandle buffer_handle, const ViewType& view_type, std::vector<DescriptorHandle>& descriptor_handles)
{
	DX12BufferViewHandle handle = 0;
	DX12BufferView view;
	view.buffer_handle = buffer_handle;
	view.buffer_view_type = view_type;
	view.buffer_descriptor_handles = descriptor_handles;

	auto it = m_buffer_to_views.find(buffer_handle);
	assert(it != m_buffer_to_views.end());

	if (HandleManager::GetHandle(HandleManager::HandleType::BUFFER_VIEW, handle))
	{
		it->second.push_back(handle);

		m_buffer_views[handle] = std::move(view);
		return handle;
	}

	it->second.push_back(m_buffer_views.size());

	m_buffer_views.push_back(std::move(view));
	return m_buffer_views.size() - 1;
}

void DX12BufferManager::UploadBufferData(DX12Core* dx12_core, DX12BufferHandle handle, void* data, uint64_t data_size, uint64_t buffer_alignment, uint64_t buffer_offset)
{
	D3D12_RANGE nothing = { 0, 0 };
	unsigned char* mapped_ptr = nullptr;
	HRESULT hr = m_upload_buffer->Map(0, &nothing, reinterpret_cast<void**>(&mapped_ptr));
	assert(SUCCEEDED(hr));

	ID3D12Resource* buffer_resource = GetBufferResource(handle);

	uint64_t source_offset = 0;

	uint64_t alignment = buffer_alignment;

	uint64_t destination_offset = ((m_upload_current_offsets[dx12_core->GetCurrentFrameInFlight()] + (alignment - 1)) & ~(alignment - 1));

	std::memcpy(mapped_ptr + destination_offset, (unsigned char*)data + source_offset, data_size);
	destination_offset += data_size;

	source_offset = ((m_upload_current_offsets[dx12_core->GetCurrentFrameInFlight()] + (alignment - 1)) & ~(alignment - 1));
	dx12_core->GetCommandList()->CopyBufferRegion(buffer_resource, m_upload_buffer.Get(), buffer_offset, source_offset, data_size);

	m_upload_buffer->Unmap(0, nullptr);

	//Transition the buffer to common state
	dx12_core->GetCommandList()->TransitionResource(buffer_resource, ResourceState::COMMON, ResourceState::COPY_DEST);

	m_upload_current_offsets[dx12_core->GetCurrentFrameInFlight()] = destination_offset;
}

DX12BufferManager::DX12BufferManager(DX12Core* dx12_core)
{
	const uint32_t buffer_size = m_fixed_buffer_size * DX12Core::GetFramesInFlight();

	m_upload_current_offsets.resize(dx12_core->GetFramesInFlight());
	for (uint32_t i = 0; i < m_upload_current_offsets.size(); ++i)
		m_upload_current_offsets[i] = i * m_fixed_buffer_size;

	//Create upload heap and buffer
	D3D12_HEAP_PROPERTIES properties;
	properties.Type = D3D12_HEAP_TYPE_UPLOAD;
	properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	properties.CreationNodeMask = 0;
	properties.VisibleNodeMask = 0;

	D3D12_HEAP_DESC heap_desc;
	heap_desc.SizeInBytes = buffer_size;
	heap_desc.Properties = properties;
	heap_desc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
	//Only allow buffers for this heap
	heap_desc.Flags = D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS;

	HRESULT hr = dx12_core->GetDevice()->CreateHeap(&heap_desc, IID_PPV_ARGS(m_upload_heap.GetAddressOf()));
	assert(SUCCEEDED(hr));

	D3D12_RESOURCE_DESC buffer_desc;
	buffer_desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	buffer_desc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
	buffer_desc.Width = buffer_size;
	buffer_desc.Height = 1;
	buffer_desc.DepthOrArraySize = 1;
	buffer_desc.MipLevels = 1;
	buffer_desc.Format = DXGI_FORMAT_UNKNOWN;
	buffer_desc.SampleDesc.Count = 1;
	buffer_desc.SampleDesc.Quality = 0;
	buffer_desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	buffer_desc.Flags = D3D12_RESOURCE_FLAG_NONE;

	hr = dx12_core->GetDevice()->CreatePlacedResource(m_upload_heap.Get(), 0, &buffer_desc, D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr, IID_PPV_ARGS(m_upload_buffer.GetAddressOf()));
	assert(SUCCEEDED(hr));

	m_shader_bindable_view = dx12_core->GetDescriptorManager()->GetDescriptorChunk(dx12_core, DescriptorHeapTypes::SHADERBINDABLE_VIEW, 10000);

	D3D12MA::ALLOCATOR_DESC allocator_desc{};
	allocator_desc.pDevice = dx12_core->GetDevice();
	allocator_desc.pAdapter = dx12_core->GetAdapter();

	hr = D3D12MA::CreateAllocator(&allocator_desc, m_buffer_allocator.GetAddressOf());
	assert(SUCCEEDED(hr));
}

DX12BufferManager::~DX12BufferManager()
{
	m_buffers.clear();
	m_buffer_views.clear();
}

DX12BufferHandle DX12BufferManager::AddBuffer(DX12Core* dx12_core, void* data, uint64_t element_size, uint64_t nr_of_elements, const BufferType& buffer_type)
{
	DX12BufferHandle handle = AddBuffer(dx12_core, element_size, nr_of_elements, buffer_type);

	UploadBufferData(dx12_core, handle, data, element_size * nr_of_elements, (uint64_t)element_size, 0);

	return handle;
}

DX12BufferHandle DX12BufferManager::AddBuffer(DX12Core* dx12_core, uint64_t element_size, uint64_t nr_of_elements, const BufferType& buffer_type)
{
	const int alignment = 256;

	uint64_t aligned_buffer_size = ((element_size * nr_of_elements + (alignment - 1)) & ~(alignment - 1));
	uint64_t buffer_size = aligned_buffer_size;

	uint64_t real_nr_of_elements = nr_of_elements;
	if (buffer_type == BufferType::MODIFIABLE_BUFFER)
	{
		real_nr_of_elements = nr_of_elements * DX12Core::GetFramesInFlight();

		buffer_size *= DX12Core::GetFramesInFlight();
	}

	D3D12_RESOURCE_DESC desc;
	desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	desc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
	desc.Width = buffer_size;
	desc.Height = 1;
	desc.DepthOrArraySize = 1;
	desc.MipLevels = 1;
	desc.Format = DXGI_FORMAT_UNKNOWN;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	desc.Flags = D3D12_RESOURCE_FLAG_NONE;

	D3D12_HEAP_PROPERTIES heap_properties;
	ZeroMemory(&heap_properties, sizeof(heap_properties));
	heap_properties.Type = D3D12_HEAP_TYPE_DEFAULT;

	Microsoft::WRL::ComPtr<ID3D12Resource> buffer;
	Microsoft::WRL::ComPtr<D3D12MA::Allocation> buffer_allocation = nullptr;

	D3D12MA::ALLOCATION_DESC allocation_desc{};
	allocation_desc.HeapType = D3D12_HEAP_TYPE_DEFAULT;
	HRESULT hr = m_buffer_allocator->CreateResource(&allocation_desc, &desc, D3D12_RESOURCE_STATE_COMMON, nullptr, buffer_allocation.GetAddressOf(), IID_PPV_ARGS(buffer.GetAddressOf()));
	assert(SUCCEEDED(hr));

	DX12BufferHandle handle = AddBuffer(buffer, buffer_allocation, ResourceState::COPY_DEST, (uint64_t)element_size, (uint64_t)nr_of_elements, buffer_type, real_nr_of_elements, aligned_buffer_size);

	return handle;
}

DX12BufferViewHandle DX12BufferManager::AddView(DX12Core* dx12_core, DX12BufferHandle buffer_handle, const ViewType& view_type)
{
	DX12Buffer* buffer = nullptr;
	D3D12_SHADER_RESOURCE_VIEW_DESC shader_resource_view_desc = {};
	D3D12_CONSTANT_BUFFER_VIEW_DESC constant_buffer_view_desc = {};
	DescriptorHandle descriptor_handle;
	std::vector<DescriptorHandle> descriptor_handles;

	uint64_t total_descriptors_needed = 1;

	switch (view_type)
	{
	case ViewType::SHADER_RESOURCE_VIEW:

		buffer = GetDX12Buffer(buffer_handle);

		if (buffer->buffer_type == BufferType::MODIFIABLE_BUFFER)
			total_descriptors_needed = (uint64_t)DX12Core::GetFramesInFlight();

		for (uint64_t i = 0; i < total_descriptors_needed; ++i)
		{
			descriptor_handle = m_shader_bindable_view.AddDescriptor();

			//Create shader resource view
			shader_resource_view_desc.Format = DXGI_FORMAT_UNKNOWN;
			shader_resource_view_desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
			shader_resource_view_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			shader_resource_view_desc.Buffer.NumElements = (UINT)buffer->nr_of_elements;
			shader_resource_view_desc.Buffer.FirstElement = i * buffer->nr_of_elements;
			shader_resource_view_desc.Buffer.StructureByteStride = (UINT)buffer->element_size;
			shader_resource_view_desc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
			dx12_core->GetDevice()->CreateShaderResourceView(buffer->buffer_resource.Get(), &shader_resource_view_desc, descriptor_handle.cpu_handle);

			descriptor_handles.push_back(descriptor_handle);
		}
		break;

	case ViewType::CONSTANT_BUFFER_VIEW:
		buffer = GetDX12Buffer(buffer_handle);

		if (buffer->buffer_type == BufferType::MODIFIABLE_BUFFER)
			total_descriptors_needed = (uint64_t)DX12Core::GetFramesInFlight();

		for (uint64_t i = 0; i < total_descriptors_needed; ++i)
		{
			descriptor_handle = m_shader_bindable_view.AddDescriptor();

			//Create constant buffer view
			constant_buffer_view_desc.BufferLocation = buffer->buffer_resource->GetGPUVirtualAddress() + buffer->aligned_buffer_size * i;
			constant_buffer_view_desc.SizeInBytes = (UINT)(buffer->aligned_buffer_size);
			dx12_core->GetDevice()->CreateConstantBufferView(&constant_buffer_view_desc, descriptor_handle.cpu_handle);

			descriptor_handles.push_back(descriptor_handle);
		}
		break;

	default:
		assert(false);
		break;
	}

	return AddView(buffer_handle, view_type, descriptor_handles);
}

DX12BufferViewHandle DX12BufferManager::AddView(DX12Core* dx12_core, const DX12BufferSubAllocation& buffer_sub_handle, const ViewType& view_type)
{
	DX12Buffer* buffer = nullptr;
	DescriptorHandle descriptor_handle;
	std::vector<DescriptorHandle> descriptor_handles;

	D3D12_SHADER_RESOURCE_VIEW_DESC shader_resource_view_desc = {};
	D3D12_CONSTANT_BUFFER_VIEW_DESC constant_buffer_view_desc = {};

	uint64_t total_descriptors_needed = 1;
	switch (view_type)
	{
	//case ViewType::SHADER_RESOURCE_VIEW:
	//	descriptor_handle = m_shader_bindable_view.AddDescriptor();

	//	buffer = GetDX12Buffer(buffer_sub_handle.handle);
	//	//Create shader resource view
	//	shader_resource_view_desc.Format = DXGI_FORMAT_UNKNOWN;
	//	shader_resource_view_desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	//	shader_resource_view_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	//	shader_resource_view_desc.Buffer.NumElements = (UINT)buffer->nr_of_elements;
	//	shader_resource_view_desc.Buffer.FirstElement = 0;
	//	shader_resource_view_desc.Buffer.StructureByteStride = (UINT)buffer->element_size;
	//	shader_resource_view_desc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
	//	dx12_core->GetDevice()->CreateShaderResourceView(buffer->buffer_resource.Get(), &shader_resource_view_desc, descriptor_handle.cpu_handle);
	//	break;
	case ViewType::CONSTANT_BUFFER_VIEW:
		if (buffer_sub_handle.buffer_type == BufferType::MODIFIABLE_BUFFER)
			total_descriptors_needed = (uint64_t)DX12Core::GetFramesInFlight();
		
		for (uint64_t i = 0; i < total_descriptors_needed; ++i)
		{
			descriptor_handle = m_shader_bindable_view.AddDescriptor();

			buffer = GetDX12Buffer(buffer_sub_handle.handle);
			//Create constant buffer view
			constant_buffer_view_desc.BufferLocation = buffer->buffer_resource->GetGPUVirtualAddress() + buffer_sub_handle.offset + i * buffer_sub_handle.size;
			constant_buffer_view_desc.SizeInBytes = (UINT)buffer_sub_handle.size;
			dx12_core->GetDevice()->CreateConstantBufferView(&constant_buffer_view_desc, descriptor_handle.cpu_handle);

			descriptor_handles.push_back(descriptor_handle);
		}
		break;
	default:
		assert(false);
		break;
	}

	return AddView(buffer_sub_handle.handle, view_type, descriptor_handles);
}

void DX12BufferManager::UploadData(DX12Core* dx12_core, DX12BufferHandle buffer_handle, void* data, uint64_t element_size, uint64_t nr_of_elements)
{
	uint64_t offset = 0;
	if (BufferType::MODIFIABLE_BUFFER == GetDX12Buffer(buffer_handle)->buffer_type && DX12Core::GetFramesInFlight() > 1)
	{
		uint64_t index = (uint64_t)dx12_core->GetCurrentBackBufferIndex();
		offset = index * element_size * nr_of_elements;
		uint64_t val1 = offset + element_size * nr_of_elements;
		uint64_t val2 = nr_of_elements * DX12Core::GetFramesInFlight() * element_size;
		assert(val1 <= val2);
	}

	UploadBufferData(dx12_core, buffer_handle, data, element_size * nr_of_elements, element_size, offset);
}

void DX12BufferManager::UploadData(DX12Core* dx12_core, const DX12BufferSubAllocation& buffer_sub_allocation, void* data, uint64_t data_size)
{
	uint64_t offset = 0;
	if (BufferType::MODIFIABLE_BUFFER == buffer_sub_allocation.buffer_type && DX12Core::GetFramesInFlight() > 1)
	{
		uint64_t index = (uint64_t)dx12_core->GetCurrentBackBufferIndex();
		offset = index * buffer_sub_allocation.size;
		assert(offset + data_size < buffer_sub_allocation.real_size);
	}

	UploadBufferData(dx12_core, buffer_sub_allocation.handle, data, data_size, buffer_sub_allocation.size, buffer_sub_allocation.offset + offset);
}

DX12BufferView* DX12BufferManager::GetBufferView(DX12BufferViewHandle texture_view_handle)
{
	return &m_buffer_views[texture_view_handle];
}

ID3D12Resource* DX12BufferManager::GetBufferResource(DX12BufferHandle handle)
{
	return m_buffers[handle].buffer_resource.Get();
}

void DX12BufferManager::FreeBuffer(DX12BufferHandle& buffer_handle)
{
	auto it = m_buffer_to_views.find(buffer_handle);

	assert(it != m_buffer_to_views.end());

	//Clear views
	for (uint32_t i = 0; i < it->second.size(); ++i)
	{
		DX12BufferView* view = GetBufferView(it->second[i]);

		//Clear view
		//Clear all sub views
		for (uint32_t view_index = 0; view_index < view->buffer_descriptor_handles.size(); ++view_index)
			m_shader_bindable_view.RemoveDescriptor(view->buffer_descriptor_handles[view_index]);

		m_buffer_views[it->second[i]] = {};
		HandleManager::FreeHandle(HandleManager::HandleType::BUFFER_VIEW, it->second[i]);

		it->second.erase(it->second.begin() + i);

		--i;
	}

	m_buffer_to_views.erase(buffer_handle);

	m_buffers[buffer_handle] = {};
	HandleManager::FreeHandle(HandleManager::HandleType::BUFFER, buffer_handle);
}

void DX12BufferManager::FreeView(DX12BufferViewHandle& view_handle)
{
	DX12BufferView* view = GetBufferView(view_handle);
	auto it = m_buffer_to_views.find(view->buffer_handle);

	assert(it != m_buffer_to_views.end());

	for (uint32_t i = 0; i < it->second.size(); ++i)
	{
		if (it->second[i] == view_handle)
		{
			it->second.erase(it->second.begin() + i);

			//Clear view
			//Clear all sub views
			for (uint32_t view_index = 0; view_index < view->buffer_descriptor_handles.size(); ++view_index)
				m_shader_bindable_view.RemoveDescriptor(view->buffer_descriptor_handles[view_index]);

			m_buffer_views[view_handle] = {};
			HandleManager::FreeHandle(HandleManager::HandleType::BUFFER_VIEW, view_handle);
			return;
		}
	}

	assert(false);
}

void DX12BufferManager::ResetUploadBuffer(DX12Core* dx12_core)
{
	if (DX12Core::GetFramesInFlight() > 1)
		m_upload_current_offsets[dx12_core->GetCurrentBackBufferIndex()] = dx12_core->GetCurrentBackBufferIndex() * m_fixed_buffer_size;
	else
		m_upload_current_offsets[0] = 0;
}

const DescriptorHandle* DX12BufferManager::GetDescriptorHandle(DX12Core* dx12_core, DX12BufferViewHandle buffer_view_handle)
{
	uint32_t get_descriptor_index = dx12_core->GetCurrentBackBufferIndex();
	DX12BufferView* buffer_view = GetBufferView(buffer_view_handle);
	if (buffer_view->buffer_descriptor_handles.size() - 1 < get_descriptor_index)
		get_descriptor_index = 0;

	return &buffer_view->buffer_descriptor_handles[get_descriptor_index];
}
