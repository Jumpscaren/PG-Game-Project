#include "pch.h"
#include "DX12ResourceDestroyer.h"
#include "DX12Core.h"

void DX12ResourceDestroyer::FreeResources(DX12Core* dx12_core)
{
	for (auto& buffer_handle : m_buffer_handles[dx12_core->GetCurrentFrameInFlight()])
	{
		dx12_core->GetBufferManager()->FreeBuffer(buffer_handle);
	}

	for (auto& buffer_view_handle : m_buffer_view_handles[dx12_core->GetCurrentFrameInFlight()])
	{
		dx12_core->GetBufferManager()->FreeView(buffer_view_handle);
	}

	for (auto& texture_handle : m_texture_handles[dx12_core->GetCurrentFrameInFlight()])
	{
		dx12_core->GetTextureManager()->FreeTexture(texture_handle);
	}

	for (auto& texture_view_handle : m_texture_view_handles[dx12_core->GetCurrentFrameInFlight()])
	{
		dx12_core->GetTextureManager()->FreeView(texture_view_handle);
	}

	m_buffer_handles[dx12_core->GetCurrentFrameInFlight()].clear();
	m_buffer_view_handles[dx12_core->GetCurrentFrameInFlight()].clear();
	m_texture_handles[dx12_core->GetCurrentFrameInFlight()].clear();
	m_texture_view_handles[dx12_core->GetCurrentFrameInFlight()].clear();
}

DX12ResourceDestroyer::DX12ResourceDestroyer()
{
	m_buffer_handles.resize(DX12Core::GetFramesInFlight());
	m_buffer_view_handles.resize(DX12Core::GetFramesInFlight());
	m_texture_handles.resize(DX12Core::GetFramesInFlight());
	m_texture_view_handles.resize(DX12Core::GetFramesInFlight());
}

DX12ResourceDestroyer::~DX12ResourceDestroyer()
{
}

void DX12ResourceDestroyer::FreeBuffer(DX12Core* dx12_core, DX12BufferHandle buffer_handle)
{
	m_buffer_handles[dx12_core->GetCurrentFrameInFlight()].push_back(buffer_handle);
}

void DX12ResourceDestroyer::FreeBufferView(DX12Core* dx12_core, DX12BufferViewHandle buffer_view_handle)
{
	m_buffer_view_handles[dx12_core->GetCurrentFrameInFlight()].push_back(buffer_view_handle);
}

void DX12ResourceDestroyer::FreeTexture(DX12Core* dx12_core, DX12TextureHandle texture_handle)
{
	m_texture_handles[dx12_core->GetCurrentFrameInFlight()].push_back(texture_handle);
}

void DX12ResourceDestroyer::FreeTextureView(DX12Core* dx12_core, DX12TextureViewHandle texture_view_handle)
{
	m_texture_view_handles[dx12_core->GetCurrentFrameInFlight()].push_back(texture_view_handle);
}
