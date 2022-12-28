#include "pch.h"
#include "DX12DescriptorManager.h"

void DX12DescriptorManager::InitDescriptorManager(DX12Core* dx12_core)
{
	m_shaderbindable = std::make_unique<DX12DescriptorHeap>();
	m_shaderbindable->InitDescriptorHeap(dx12_core, DescriptorHeapTypes::SHADERBINDABLE_VIEW, 30000, true);

	m_rendertargetview = std::make_unique<DX12DescriptorHeap>();
	m_rendertargetview->InitDescriptorHeap(dx12_core, DescriptorHeapTypes::RENDERTARGET_VIEW, 10, false);

	m_depthstencilview = std::make_unique<DX12DescriptorHeap>();
	m_depthstencilview->InitDescriptorHeap(dx12_core, DescriptorHeapTypes::DEPTHSTENCIL_VIEW, 2, false);
}

DX12DescriptorChunk DX12DescriptorManager::GetDescriptorChunk(DX12Core* dx12_core, const DescriptorHeapTypes& descriptor_type, uint32_t descriptors)
{
	switch (descriptor_type)
	{
	case DescriptorHeapTypes::RENDERTARGET_VIEW:
		return m_rendertargetview->GetDescriptorChunk(dx12_core, descriptor_type, descriptors);
	case DescriptorHeapTypes::DEPTHSTENCIL_VIEW:
		return m_depthstencilview->GetDescriptorChunk(dx12_core, descriptor_type, descriptors);
	case DescriptorHeapTypes::SHADERBINDABLE_VIEW:
		return m_shaderbindable->GetDescriptorChunk(dx12_core, descriptor_type, descriptors);
	default:
		assert(false);
	}
	return DX12DescriptorChunk();
}

DX12DescriptorHeap* DX12DescriptorManager::GetShaderBindable()
{
	return m_shaderbindable.get();
}
