#pragma once
#include "DX12DescriptorChunk.h"
#include "BufferTypes.h"
#include "DX12MemAllocInclude.h"

class DX12Core;
class DX12CommandList;
class DX12ResourceDestroyer;

class DX12BufferManager
{
	friend DX12Core;
	friend DX12CommandList;
	friend DX12ResourceDestroyer;

private:
	Microsoft::WRL::ComPtr<ID3D12Heap> m_upload_heap;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_upload_buffer;
	std::vector<uint64_t> m_upload_current_offsets;

	std::vector<DX12Buffer> m_buffers;
	std::vector<DX12BufferView> m_buffer_views;

	DX12DescriptorChunk m_shader_bindable_view;

	Microsoft::WRL::ComPtr<D3D12MA::Allocator> m_buffer_allocator;

	qr::unordered_map<DX12BufferHandle, std::vector<DX12BufferViewHandle>> m_buffer_to_views;

	static constexpr uint32_t m_fixed_buffer_size = 10'000'000;

private:
	DX12Buffer* GetDX12Buffer(DX12BufferHandle handle);
	DX12BufferView* GetBufferView(DX12BufferViewHandle texture_view_handle);
	ID3D12Resource* GetBufferResource(DX12BufferHandle handle);
	DX12BufferHandle AddBuffer(Microsoft::WRL::ComPtr<ID3D12Resource> buffer, Microsoft::WRL::ComPtr<D3D12MA::Allocation> buffer_allocation, const ResourceState& state, uint64_t element_size, uint64_t nr_of_elements, const BufferType& buffer_type, uint64_t real_nr_of_elements, uint64_t aligned_buffer_size);
	DX12BufferViewHandle AddView(DX12BufferHandle buffer_handle, const ViewType& view_type, std::vector<DescriptorHandle>& descriptor_handles);
	void UploadBufferData(DX12Core* dx12_core, DX12BufferHandle handle, void* data , uint64_t data_size, uint64_t buffer_alignment, uint64_t buffer_offset);

	void ResetUploadBuffer(DX12Core* dx12_core);

	const DescriptorHandle* GetDescriptorHandle(DX12Core* dx12_core, DX12BufferViewHandle buffer_view_handle);

	void FreeBuffer(DX12BufferHandle& buffer_handle);
	void FreeView(DX12BufferViewHandle& view_handle);

public:
	DX12BufferManager() = delete;
	DX12BufferManager(DX12Core* dx12_core);
	~DX12BufferManager();

	DX12BufferHandle AddBuffer(DX12Core* dx12_core, void* data, uint64_t element_size, uint64_t nr_of_elements, const BufferType& buffer_type);
	DX12BufferHandle AddBuffer(DX12Core* dx12_core, uint64_t element_size, uint64_t nr_of_elements, const BufferType& buffer_type);
	DX12BufferViewHandle AddView(DX12Core* dx12_core, DX12BufferHandle buffer_handle, const ViewType& view_type);
	DX12BufferViewHandle AddView(DX12Core* dx12_core, const DX12BufferSubAllocation& buffer_sub_handle, const ViewType& view_type);

	void UploadData(DX12Core* dx12_core, DX12BufferHandle buffer_handle, void* data, uint64_t element_size, uint64_t nr_of_elements);
	void UploadData(DX12Core* dx12_core, const DX12BufferSubAllocation& buffer_sub_allocation, void* data, uint64_t data_size);
};

