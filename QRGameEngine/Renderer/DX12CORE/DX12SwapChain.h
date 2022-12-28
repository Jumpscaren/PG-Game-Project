#pragma once
#include "TextureTypes.h"

class DX12Core;

class DX12SwapChain
{
	friend DX12Core;

private:
	Microsoft::WRL::ComPtr<IDXGISwapChain3> m_swapchain;
	uint32_t m_backbuffer_count = 0;
	uint32_t m_backbuffer_width = 0;
	uint32_t m_backbuffer_height = 0;
	std::vector<DX12TextureHandle> m_backbuffers;
	std::vector<DX12TextureViewHandle> m_rtv_views;

private:
	uint32_t GetCurrentBackBufferIndex() const;

public:
	DX12SwapChain() = default;
	void InitSwapChain(DX12Core* dx12_core, uint32_t backbuffer_count);
	void Present();
	DX12TextureHandle GetBackbufferTexture();
	DX12TextureViewHandle GetBackbufferView();
};

