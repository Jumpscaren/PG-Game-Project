#pragma once
#include "TextureTypes.h"
#include "DX12DescriptorChunk.h"
#include "DX12MemAllocInclude.h"

class DX12Core;
class DX12CommandList;
class DX12ResourceDestroyer;

struct TextureInfo;

class DX12TextureManager
{
friend DX12Core;
friend DX12CommandList;
friend DX12ResourceDestroyer;

private:
	Microsoft::WRL::ComPtr<ID3D12Heap> m_upload_heap;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_upload_buffer;
	uint64_t m_upload_current_offset;

	std::vector<DX12Texture> m_textures;
	std::vector<DX12TextureView> m_texture_views;

	DX12DescriptorChunk m_shader_bindable_view;
	DX12DescriptorChunk m_depthstencil_view;
	DX12DescriptorChunk m_rendertarget_view;

	Microsoft::WRL::ComPtr<D3D12MA::Allocator> m_texture_allocator;

	std::unordered_map<DX12TextureHandle, std::vector<DX12TextureViewHandle>> m_texture_to_views;

private:
	DX12TextureHandle AddTexture(Microsoft::WRL::ComPtr<ID3D12Resource> texture, Microsoft::WRL::ComPtr<D3D12MA::Allocation> texture_allocation, const ResourceState& state);
	DX12TextureViewHandle AddView(DX12TextureHandle texture_handle, const ViewType& view_type, const DescriptorHandle& descriptor_handle);

	void UploadTextureData(DX12Core* dx12_core, DX12TextureHandle handle, void* data, uint64_t alignment);

	ID3D12Resource* GetTextureResource(const DX12TextureHandle& texture_handle);
	DX12Texture* GetTexture(DX12TextureHandle texture_handle);
	DX12TextureView* GetTextureView(DX12TextureViewHandle texture_view_handle);

	void FreeTexture(DX12TextureHandle& texture_handle);
	void FreeView(DX12TextureViewHandle& view_handle);

	void ResetUploadBuffer();

public:
	DX12TextureManager() = delete;
	DX12TextureManager(DX12Core* dx12_core);
	~DX12TextureManager();

	DX12TextureHandle AddSwapchainTexture(Microsoft::WRL::ComPtr<ID3D12Resource> backbuffer);
	DX12TextureHandle AddTexture(DX12Core* dx12_core, uint32_t texture_width, uint32_t texture_height, TextureFlags texture_flag);
	DX12TextureHandle AddTexture(DX12Core* dx12_core, TextureInfo* texture_data, TextureFlags texture_flag);
	DX12TextureViewHandle AddView(DX12Core* dx12_core, DX12TextureHandle texture_handle, const ViewType& view_type);

	uint32_t ConvertTextureViewHandleToGPUTextureViewHandle(DX12TextureViewHandle texture_view_handle);
};

