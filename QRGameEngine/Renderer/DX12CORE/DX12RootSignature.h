#pragma once
#include "SamplerTypes.h"
#include "HelpTypes.h"
class DX12Core;
class DX12Pipeline;
class DX12CommandList;

class DX12RootSignature
{
	friend DX12Pipeline;
	friend DX12CommandList;

private:
	Microsoft::WRL::ComPtr<ID3D12RootSignature> m_root_signature;
	std::vector<D3D12_ROOT_PARAMETER> m_root_parameters;
	std::vector<std::vector<D3D12_DESCRIPTOR_RANGE>> m_ranges;
	std::vector<D3D12_STATIC_SAMPLER_DESC> m_static_samplers;

private:
	D3D12_STATIC_SAMPLER_DESC GetSamplerDesc(const SamplerTypes& sampler_type);
	ID3D12RootSignature* GetRootSignature();
	void ClearParameters();

public:
	DX12RootSignature() = default;
	~DX12RootSignature();
	DX12RootSignature& AddStruturedBuffer(DX12Core* dx12_core, const ShaderVisibility& shader_visibility, uint32_t shader_binding_index, uint32_t shader_space = 0);
	DX12RootSignature& AddConstantBuffer(DX12Core* dx12_core, const ShaderVisibility& shader_visibility, uint32_t shader_binding_index, uint32_t shader_space = 0);
	DX12RootSignature& AddStaticSampler(DX12Core* dx12_core, const SamplerTypes& sampler_type, uint32_t shader_binding_index, uint32_t shader_space = 0);
	DX12RootSignature& AddConstant(DX12Core* dx12_core, const ShaderVisibility& shader_visibility, uint32_t shader_binding_index, uint32_t shader_space = 0);
	void InitRootSignature(DX12Core* dx12_core);
};

