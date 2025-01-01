#include "pch.h"
#include "DX12TextureManager.h"
#include "DX12Core.h"
#include "../Helpers/HandleManager.h"
#include "Asset/AssetManager.h"

using Microsoft::WRL::ComPtr;

void DX12TextureManager::FreeTexture(DX12TextureHandle& texture_handle)
{
	auto it = m_texture_to_views.find(texture_handle);

	assert(it != m_texture_to_views.end());

	//Clear views
	for (uint32_t i = 0; i < it->second.size(); ++i)
	{
		DX12TextureView* view = GetTextureView(it->second[i]);

		//Clear view
		switch (view->texture_view_type)
		{
		case ViewType::SHADER_RESOURCE_VIEW:
			m_shader_bindable_view.RemoveDescriptor(view->texture_descriptor_handle);
			break;
		case ViewType::RENDER_TARGET_VIEW:
			m_rendertarget_view.RemoveDescriptor(view->texture_descriptor_handle);
			break;
		case ViewType::DEPTH_STENCIL_VIEW:
			m_depthstencil_view.RemoveDescriptor(view->texture_descriptor_handle);
			break;
		}
		m_texture_views[it->second[i]] = {};
		HandleManager::FreeHandle(HandleManager::HandleType::TEXTURE_VIEW, it->second[i]);

		it->second.erase(it->second.begin() + i);

		--i;
	}

	m_texture_to_views.erase(texture_handle);

	m_textures[texture_handle] = {};
	HandleManager::FreeHandle(HandleManager::HandleType::TEXTURE, texture_handle);
}

void DX12TextureManager::FreeView(DX12TextureViewHandle& view_handle)
{
	DX12TextureView* view = GetTextureView(view_handle);
	auto it = m_texture_to_views.find(view->texture_handle);

	assert(it != m_texture_to_views.end());

	for (uint32_t i = 0; i < it->second.size(); ++i)
	{
		if (it->second[i] == view_handle)
		{
			it->second.erase(it->second.begin() + i);

			//Clear view
			switch (view->texture_view_type)
			{
			case ViewType::SHADER_RESOURCE_VIEW:
				m_shader_bindable_view.RemoveDescriptor(view->texture_descriptor_handle);
				break;
			case ViewType::RENDER_TARGET_VIEW:
				m_rendertarget_view.RemoveDescriptor(view->texture_descriptor_handle);
				break;
			case ViewType::DEPTH_STENCIL_VIEW:
				m_depthstencil_view.RemoveDescriptor(view->texture_descriptor_handle);
				break;
			}
			m_texture_views[view_handle] = {};
			HandleManager::FreeHandle(HandleManager::HandleType::BUFFER_VIEW, view_handle);
			return;
		}
	}

	assert(false);
}

void DX12TextureManager::ResetUploadBuffer()
{
	m_upload_current_offset = 0;
}

uint32_t DX12TextureManager::ConvertTextureViewHandleToGPUTextureViewHandle(DX12TextureViewHandle texture_view_handle)
{
	return GetTextureView(texture_view_handle)->texture_descriptor_handle.descriptor_offset;
}

DX12TextureHandle DX12TextureManager::AddTexture(Microsoft::WRL::ComPtr<ID3D12Resource> texture, Microsoft::WRL::ComPtr<D3D12MA::Allocation> texture_allocation, const ResourceState& state)
{
	uint64_t handle = 0;
	if (HandleManager::GetHandle(HandleManager::HandleType::TEXTURE, handle))
	{
		m_texture_to_views.insert({ handle, {} });

		m_textures[handle] = { texture, texture_allocation };
		return handle;
	}

	m_texture_to_views.insert({ m_textures.size(), {} });
	m_textures.push_back({ texture, texture_allocation });
	return m_textures.size() - 1;
}

DX12TextureViewHandle DX12TextureManager::AddView(DX12TextureHandle texture_handle, const ViewType& view_type, const DescriptorHandle& descriptor_handle)
{
	DX12TextureViewHandle handle = 0;
	DX12TextureView view;
	view.texture_handle = texture_handle;
	view.texture_view_type = view_type;
	view.texture_descriptor_handle = descriptor_handle;

	auto it = m_texture_to_views.find(texture_handle);
	assert(it != m_texture_to_views.end());

	if (HandleManager::GetHandle(HandleManager::HandleType::TEXTURE_VIEW, handle))
	{
		it->second.push_back(handle);

		m_texture_views[handle] = view;
		return handle;
	}

	it->second.push_back(m_texture_views.size());

	m_texture_views.push_back(view);
	return m_texture_views.size() - 1;
}

void DX12TextureManager::UploadTextureData(DX12Core* dx12_core, DX12TextureHandle handle, void* data, uint64_t alignment)
{
	int subresource = 0;

	D3D12_RANGE nothing = { 0, 0 };
	unsigned char* mapped_ptr = nullptr;
	HRESULT hr = m_upload_buffer->Map(0, &nothing, reinterpret_cast<void**>(&mapped_ptr));
	assert(SUCCEEDED(hr));

	ID3D12Resource* texture_resource = GetTextureResource(handle);

	D3D12_RESOURCE_DESC resource_desc = texture_resource->GetDesc();
	D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint;
	UINT nr_of_rows = 0;
	UINT64 row_size_in_bytes = 0;
	UINT64 total_bytes = 0;
	dx12_core->GetDevice()->GetCopyableFootprints(&resource_desc, subresource, 1, 0, &footprint, &nr_of_rows,
		&row_size_in_bytes, &total_bytes);

	uint64_t source_offset = 0;
	size_t destination_offset = ((m_upload_current_offset + (alignment - 1)) & ~(alignment - 1));//AlignTextureAdress(m_current_upload_offset, alignment);

	for (UINT row = 0; row < nr_of_rows; ++row)
	{
		std::memcpy(mapped_ptr + destination_offset, (unsigned char*)data + source_offset, row_size_in_bytes);
		source_offset += row_size_in_bytes;
		destination_offset += footprint.Footprint.RowPitch;
	}

	D3D12_TEXTURE_COPY_LOCATION destination;
	destination.pResource = texture_resource;
	destination.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
	destination.SubresourceIndex = subresource;
	D3D12_TEXTURE_COPY_LOCATION source;
	source.pResource = m_upload_buffer.Get();
	source.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
	source.PlacedFootprint.Offset = ((m_upload_current_offset + (alignment - 1)) & ~(alignment - 1));//AlignTextureAdress(m_current_upload_offset, alignment);
	source.PlacedFootprint.Footprint.Width = footprint.Footprint.Width;
	source.PlacedFootprint.Footprint.Height = footprint.Footprint.Height;
	source.PlacedFootprint.Footprint.Depth = footprint.Footprint.Depth;
	source.PlacedFootprint.Footprint.RowPitch = footprint.Footprint.RowPitch;
	source.PlacedFootprint.Footprint.Format = resource_desc.Format;

	//Wait until the gpu is completed
	dx12_core->GetCommandList()->Wait(dx12_core);
	dx12_core->GetCommandList()->Reset();

	dx12_core->GetCommandList()->TransitionResource(texture_resource, ResourceState::COPY_DEST, ResourceState::COMMON);
	dx12_core->GetCommandList()->CopyTextureRegion(&destination, &source);
	m_upload_buffer->Unmap(0, nullptr);

	dx12_core->GetCommandList()->TransitionResource(texture_resource, ResourceState::COMMON, ResourceState::COPY_DEST);

	//Throttle the GPU to upload texture data (which requires less memory for CPU buffer)
	dx12_core->GetCommandList()->Execute(dx12_core, dx12_core->GetGraphicsCommandQueue());
	dx12_core->GetCommandList()->Signal(dx12_core, dx12_core->GetGraphicsCommandQueue());
	dx12_core->GetCommandList()->Wait(dx12_core);
	dx12_core->GetCommandList()->Reset();

	//m_upload_current_offset = destination_offset;
}

DX12TextureManager::DX12TextureManager(DX12Core* dx12_core)
{
	const UINT upload_buffer_size = 20'000'000;

	m_upload_current_offset = 0;

	//Create upload heap and buffer
	D3D12_HEAP_PROPERTIES properties;
	properties.Type = D3D12_HEAP_TYPE_UPLOAD;
	properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	properties.CreationNodeMask = 0;
	properties.VisibleNodeMask = 0;

	D3D12_HEAP_DESC heap_desc;
	heap_desc.SizeInBytes = upload_buffer_size;
	heap_desc.Properties = properties;
	heap_desc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
	heap_desc.Flags = D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS;

	HRESULT hr = dx12_core->GetDevice()->CreateHeap(&heap_desc, IID_PPV_ARGS(m_upload_heap.GetAddressOf()));
	assert(SUCCEEDED(hr));

	D3D12_RESOURCE_DESC buffer_desc;
	buffer_desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	buffer_desc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
	buffer_desc.Width = upload_buffer_size;
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
	m_depthstencil_view = dx12_core->GetDescriptorManager()->GetDescriptorChunk(dx12_core, DescriptorHeapTypes::DEPTHSTENCIL_VIEW, 1);
	m_rendertarget_view = dx12_core->GetDescriptorManager()->GetDescriptorChunk(dx12_core, DescriptorHeapTypes::RENDERTARGET_VIEW, 10);

	D3D12MA::ALLOCATOR_DESC allocator_desc{};
	allocator_desc.pDevice = dx12_core->GetDevice();
	allocator_desc.pAdapter = dx12_core->GetAdapter();
	hr = D3D12MA::CreateAllocator(&allocator_desc, m_texture_allocator.GetAddressOf());
	assert(SUCCEEDED(hr));
}

DX12TextureManager::~DX12TextureManager()
{
	m_textures.clear();
	m_texture_views.clear();
}

DX12TextureHandle DX12TextureManager::AddSwapchainTexture(Microsoft::WRL::ComPtr<ID3D12Resource> backbuffer)
{
	return AddTexture(backbuffer, nullptr, ResourceState::COMMON);
}

DX12TextureHandle DX12TextureManager::AddTexture(DX12Core* dx12_core, uint32_t texture_width, uint32_t texture_height, TextureFlags texture_flag)
{
	DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
	if (texture_flag == TextureFlags::DEPTSTENCIL_DENYSHADER_FLAG)
		format = DXGI_FORMAT_D32_FLOAT;
	else
		format = DXGI_FORMAT_R8G8B8A8_UNORM;

	D3D12_CLEAR_VALUE* clear_value = nullptr;
	D3D12_CLEAR_VALUE depth_clear_value;
	depth_clear_value.Format = format;

	if (texture_flag == TextureFlags::DEPTSTENCIL_DENYSHADER_FLAG)
	{
		depth_clear_value.DepthStencil.Depth = 1.0f;
		clear_value = &depth_clear_value;
	}

	D3D12_HEAP_PROPERTIES heap_properties;
	ZeroMemory(&heap_properties, sizeof(heap_properties));
	heap_properties.Type = D3D12_HEAP_TYPE_DEFAULT;

	D3D12_RESOURCE_DESC desc;
	desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	desc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
	desc.Width = texture_width;
	desc.Height = texture_height;
	desc.DepthOrArraySize = 1;
	desc.MipLevels = static_cast<UINT16>(1);
	desc.Format = format;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	desc.Flags = (D3D12_RESOURCE_FLAGS)texture_flag;

	ComPtr<ID3D12Resource> texture = nullptr;
	//HRESULT hr = dx12_core->GetDevice()->CreateCommittedResource(&heap_properties, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_COMMON,
	//	clear_value, IID_PPV_ARGS(texture.GetAddressOf()));
	//assert(SUCCEEDED(hr));

	ComPtr<D3D12MA::Allocation> texture_allocation = nullptr;

	D3D12MA::ALLOCATION_DESC allocation_desc{};
	allocation_desc.HeapType = D3D12_HEAP_TYPE_DEFAULT;
	HRESULT hr = m_texture_allocator->CreateResource(&allocation_desc, &desc, D3D12_RESOURCE_STATE_COMMON, clear_value, texture_allocation.GetAddressOf(), IID_PPV_ARGS(texture.GetAddressOf()));
	assert(SUCCEEDED(hr));

	return AddTexture(texture, texture_allocation, ResourceState::COMMON);
}

DX12TextureHandle DX12TextureManager::AddTexture(DX12Core* dx12_core, TextureInfo* texture_data, TextureFlags texture_flag)
{
	DX12TextureHandle texture = AddTexture(dx12_core, texture_data->width, texture_data->height, texture_flag);

	UploadTextureData(dx12_core, texture, texture_data->texture_data, D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT);

	return texture;
}

DX12TextureViewHandle DX12TextureManager::AddView(DX12Core* dx12_core, DX12TextureHandle texture_handle, const ViewType& view_type)
{
	DescriptorHandle texture_descriptor_handle;
	switch (view_type)
	{
	case ViewType::SHADER_RESOURCE_VIEW:
		texture_descriptor_handle = m_shader_bindable_view.AddDescriptor();
		dx12_core->GetDevice()->CreateShaderResourceView(GetTextureResource(texture_handle), nullptr, texture_descriptor_handle.cpu_handle);
		break;
	case ViewType::DEPTH_STENCIL_VIEW:
		texture_descriptor_handle = m_depthstencil_view.AddDescriptor();
		dx12_core->GetDevice()->CreateDepthStencilView(GetTextureResource(texture_handle), nullptr, texture_descriptor_handle.cpu_handle);
		break;
	case ViewType::RENDER_TARGET_VIEW:
		texture_descriptor_handle = m_rendertarget_view.AddDescriptor();
		dx12_core->GetDevice()->CreateRenderTargetView(GetTextureResource(texture_handle), nullptr, texture_descriptor_handle.cpu_handle);
		break;
	case ViewType::UNORDERED_ACCESS_VIEW:
		texture_descriptor_handle = m_shader_bindable_view.AddDescriptor();
		dx12_core->GetDevice()->CreateUnorderedAccessView(GetTextureResource(texture_handle), nullptr, nullptr, texture_descriptor_handle.cpu_handle);
		break;
	default:
		assert(false);
		break;
	}
	DX12TextureViewHandle view_handle = AddView(texture_handle, view_type, texture_descriptor_handle);

	return view_handle;
}

ID3D12Resource* DX12TextureManager::GetTextureResource(const DX12TextureHandle& texture_handle)
{
	return m_textures[texture_handle].texture_resource.Get();
}

DX12Texture* DX12TextureManager::GetTexture(DX12TextureHandle texture_handle)
{
	return &m_textures[texture_handle];
}

DX12TextureView* DX12TextureManager::GetTextureView(DX12TextureViewHandle texture_view_handle)
{
	return &m_texture_views[texture_view_handle];
}
