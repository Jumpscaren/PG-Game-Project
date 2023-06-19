#include "pch.h"
#include "DX12RootSignature.h"
#include "DX12Core.h"

D3D12_STATIC_SAMPLER_DESC DX12RootSignature::GetSamplerDesc(const SamplerTypes& sampler_type)
{
	//D3D12_STATIC_SAMPLER_DESC sampler_desc;
//sampler_desc.MipLODBias = 0.0f;
//sampler_desc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
//sampler_desc.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
//sampler_desc.MinLOD = 0;
//sampler_desc.MaxLOD = D3D12_FLOAT32_MAX;
//sampler_desc.Filter = sampler_filter;
//sampler_desc.AddressU = sampler_desc.AddressW = sampler_desc.AddressV = sampler_address_mode;

	//if (sampler_filter == D3D12_FILTER_ANISOTROPIC)
//	sampler_desc.MaxAnisotropy = 16;
//else
//	sampler_desc.MaxAnisotropy = 1;

	D3D12_STATIC_SAMPLER_DESC sampler_desc;

	switch (sampler_type)
	{
	case SamplerTypes::LINEAR_WRAP:
		sampler_desc.MipLODBias = 0.0f;
		sampler_desc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
		sampler_desc.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
		sampler_desc.MinLOD = 0;
		sampler_desc.MaxLOD = D3D12_FLOAT32_MAX;
		sampler_desc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
		sampler_desc.AddressU = sampler_desc.AddressW = sampler_desc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		sampler_desc.MaxAnisotropy = 1;
		break;
	default:
		assert(false);
		break;
	}
	return sampler_desc;
}

ID3D12RootSignature* DX12RootSignature::GetRootSignature()
{
	return m_root_signature.Get();
}

void DX12RootSignature::ClearParameters()
{
	m_root_parameters.clear();
	//bool g = m_root_parameters.empty();
	m_ranges.clear();
	//g = m_ranges.empty();
}

DX12RootSignature::~DX12RootSignature()
{
}

DX12RootSignature& DX12RootSignature::AddStruturedBuffer(DX12Core* dx12_core, const ShaderVisibility& shader_visibility, uint32_t shader_binding_index, uint32_t shader_space)
{
	D3D12_DESCRIPTOR_RANGE descriptor_range;
	descriptor_range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptor_range.NumDescriptors = 1;
	descriptor_range.BaseShaderRegister = shader_binding_index;
	descriptor_range.RegisterSpace = shader_space;
	descriptor_range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	std::vector<D3D12_DESCRIPTOR_RANGE> range = { descriptor_range };
	m_ranges.push_back(range);

	D3D12_ROOT_PARAMETER descriptor_table;
	descriptor_table.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	descriptor_table.ShaderVisibility = (D3D12_SHADER_VISIBILITY)shader_visibility;
	descriptor_table.DescriptorTable.NumDescriptorRanges = (UINT)(m_ranges[m_ranges.size() - 1].size());
	descriptor_table.DescriptorTable.pDescriptorRanges = m_ranges[m_ranges.size() - 1].size() != 0 ? m_ranges[m_ranges.size() - 1].data() : nullptr;

	m_root_parameters.push_back(descriptor_table);

	return *this;
}

DX12RootSignature& DX12RootSignature::AddConstantBuffer(DX12Core* dx12_core, const ShaderVisibility& shader_visibility, uint32_t shader_binding_index, uint32_t shader_space)
{
	D3D12_DESCRIPTOR_RANGE descriptor_range;
	descriptor_range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	descriptor_range.NumDescriptors = 1;
	descriptor_range.BaseShaderRegister = shader_binding_index;
	descriptor_range.RegisterSpace = shader_space;
	descriptor_range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	std::vector<D3D12_DESCRIPTOR_RANGE> range = { descriptor_range };
	m_ranges.push_back(range);

	D3D12_ROOT_PARAMETER descriptor_table;
	descriptor_table.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	descriptor_table.ShaderVisibility = (D3D12_SHADER_VISIBILITY)shader_visibility;
	descriptor_table.DescriptorTable.NumDescriptorRanges = (UINT)(m_ranges[m_ranges.size() - 1].size());
	descriptor_table.DescriptorTable.pDescriptorRanges = m_ranges[m_ranges.size() - 1].size() != 0 ? m_ranges[m_ranges.size() - 1].data() : nullptr;

	m_root_parameters.push_back(descriptor_table);

	return *this;
}

DX12RootSignature& DX12RootSignature::AddStaticSampler(DX12Core* dx12_core, const SamplerTypes& sampler_type, uint32_t shader_binding_index, uint32_t shader_space)
{
	D3D12_STATIC_SAMPLER_DESC sampler_desc = GetSamplerDesc(sampler_type);

	sampler_desc.ShaderRegister = shader_binding_index;
	sampler_desc.RegisterSpace = shader_space;
	sampler_desc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	m_static_samplers.push_back(sampler_desc);

	return *this;
}

DX12RootSignature& DX12RootSignature::AddConstant(DX12Core* dx12_core, const ShaderVisibility& shader_visibility, uint32_t shader_binding_index, uint32_t shader_space)
{
	D3D12_ROOT_PARAMETER constant;
	constant.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	constant.ShaderVisibility = (D3D12_SHADER_VISIBILITY)shader_visibility;
	constant.Constants.Num32BitValues = 1;
	constant.Constants.RegisterSpace = shader_space;
	constant.Constants.ShaderRegister = shader_binding_index;

	m_root_parameters.push_back(constant);

	return *this;
}

DX12RootSignature& DX12RootSignature::AddShaderResourceView(DX12Core* dx12_core, const ShaderVisibility& shader_visibility, uint32_t shader_binding_index, uint32_t shader_space)
{
	D3D12_DESCRIPTOR_RANGE descriptor_range;
	descriptor_range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptor_range.NumDescriptors = 1;
	descriptor_range.BaseShaderRegister = shader_binding_index;
	descriptor_range.RegisterSpace = shader_space;
	descriptor_range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	std::vector<D3D12_DESCRIPTOR_RANGE> range = { descriptor_range };
	m_ranges.push_back(range);

	D3D12_ROOT_PARAMETER descriptor_table;
	descriptor_table.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	descriptor_table.ShaderVisibility = (D3D12_SHADER_VISIBILITY)shader_visibility;
	descriptor_table.DescriptorTable.NumDescriptorRanges = (UINT)(m_ranges[m_ranges.size() - 1].size());
	descriptor_table.DescriptorTable.pDescriptorRanges = m_ranges[m_ranges.size() - 1].size() != 0 ? m_ranges[m_ranges.size() - 1].data() : nullptr;

	m_root_parameters.push_back(descriptor_table);

	return *this;
}

void DX12RootSignature::InitRootSignature(DX12Core* dx12_core)
{
	D3D12_ROOT_SIGNATURE_DESC desc;
	desc.NumParameters = static_cast<UINT>(m_root_parameters.size());
	desc.pParameters = m_root_parameters.size() == 0 ? nullptr : m_root_parameters.data();
	desc.NumStaticSamplers = static_cast<UINT>(m_static_samplers.size());
	desc.pStaticSamplers = m_static_samplers.size() == 0 ? nullptr : m_static_samplers.data();
	desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED | D3D12_ROOT_SIGNATURE_FLAG_SAMPLER_HEAP_DIRECTLY_INDEXED;

	ID3DBlob* root_signature_blob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1_0, &root_signature_blob, nullptr);

	assert(SUCCEEDED(hr));

	hr = dx12_core->GetDevice()->CreateRootSignature(0, root_signature_blob->GetBufferPointer(), root_signature_blob->GetBufferSize(),
		IID_PPV_ARGS(m_root_signature.GetAddressOf()));
	assert(SUCCEEDED(hr));

	root_signature_blob->Release();
}
