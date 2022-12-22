#include "pch.h"
#include "DX12SwapChain.h"
#include "DX12Core.h"
#include "Window.h"

using Microsoft::WRL::ComPtr;

uint32_t DX12SwapChain::GetCurrentBackBufferIndex() const
{
	return (uint32_t)m_swapchain->GetCurrentBackBufferIndex();
}

void DX12SwapChain::InitSwapChain(DX12Core* dx12_core, uint32_t backbuffer_count)
{
	m_backbuffer_count = backbuffer_count;

	DXGI_SWAP_CHAIN_DESC1 desc;
	desc.Width = 0;
	desc.Height = 0;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.Stereo = false;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	desc.BufferCount = m_backbuffer_count;
	desc.Scaling = DXGI_SCALING_STRETCH;
	desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	desc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	desc.Flags = 0;

	IDXGISwapChain1* swap_chain;
	HRESULT hr = dx12_core->GetFactory()->CreateSwapChainForHwnd(dx12_core->GetGraphicsCommandQueue()->GetCommandQueue(), dx12_core->GetWindow()->GetWindowHandle(), &desc, nullptr, nullptr, &swap_chain);
	assert(SUCCEEDED(hr));

	//Change the Swapchain from IDXGISwapChain1 to IDXGISwapChain3
	hr = swap_chain->QueryInterface(__uuidof(IDXGISwapChain3), (void**)m_swapchain.GetAddressOf());
	assert(SUCCEEDED(hr));
	swap_chain->Release();

	m_swapchain->GetDesc1(&desc);

	//Getting the size of the window
	m_backbuffer_width = desc.Width;
	m_backbuffer_height = desc.Height;

	//Get the backbuffers
	for (uint32_t i = 0; i < backbuffer_count; ++i)
	{
		ComPtr<ID3D12Resource> backbuffer = nullptr;
		HRESULT hr = m_swapchain->GetBuffer(i, IID_PPV_ARGS(backbuffer.GetAddressOf()));
		assert(SUCCEEDED(hr));
		
		m_backbuffers.push_back(dx12_core->GetTextureManager()->AddSwapchainTexture(backbuffer));

		m_rtv_views.push_back(dx12_core->GetTextureManager()->AddView(dx12_core, m_backbuffers[m_backbuffers.size()-1], ViewType::RENDER_TARGET_VIEW));
	}
}

void DX12SwapChain::Present()
{
	m_swapchain->Present(0,0);
}

DX12TextureHandle DX12SwapChain::GetBackbufferTexture()
{
	uint64_t current_backbuffer_index = m_swapchain->GetCurrentBackBufferIndex();
	return m_backbuffers[current_backbuffer_index];
}

DX12TextureViewHandle DX12SwapChain::GetBackbufferView()
{
	uint64_t current_backbuffer_index = m_swapchain->GetCurrentBackBufferIndex();
	return m_rtv_views[current_backbuffer_index];
}
