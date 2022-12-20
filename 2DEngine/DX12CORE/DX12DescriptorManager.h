#pragma once
#include "DX12DescriptorHeap.h"

class DX12Core;

class DX12DescriptorManager
{
private:
	std::unique_ptr<DX12DescriptorHeap> m_shaderbindable;
	std::unique_ptr<DX12DescriptorHeap> m_rendertargetview;
	std::unique_ptr<DX12DescriptorHeap> m_depthstencilview;
public:
	DX12DescriptorManager() = default;
	void InitDescriptorManager(DX12Core* dx12_core);

	DX12DescriptorChunk GetDescriptorChunk(DX12Core* dx12_core, const DescriptorHeapTypes& descriptor_type, uint32_t descriptors);

	DX12DescriptorHeap* GetShaderBindable();
};

