#pragma once

#include "BufferTypes.h"
#include "TextureTypes.h"

class DX12Core;

class DX12ResourceDestroyer
{
private:
	std::vector<std::vector<DX12BufferHandle>> m_buffer_handles;
	std::vector<std::vector<DX12BufferViewHandle>> m_buffer_view_handles;
	std::vector<std::vector<DX12TextureHandle>> m_texture_handles;
	std::vector<std::vector<DX12TextureViewHandle>> m_texture_view_handles;
public:
	DX12ResourceDestroyer();
	~DX12ResourceDestroyer();

	void FreeBuffer(DX12Core* dx12_core, DX12BufferHandle buffer_handle);
	void FreeBufferView(DX12Core* dx12_core, DX12BufferViewHandle buffer_view_handle);
	void FreeTexture(DX12Core* dx12_core, DX12TextureHandle texture_handle);
	void FreeTextureView(DX12Core* dx12_core, DX12TextureViewHandle texture_view_handle);

	void FreeResources(DX12Core* dx12_core);
};

