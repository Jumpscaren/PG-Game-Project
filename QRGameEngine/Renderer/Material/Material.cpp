#include "pch.h"
#include "Material.h"
#include "Renderer/DX12CORE/DX12Core.h"

void Material::Initialize(DX12Core* dx12_core)
{
	m_root_signature
		.AddStaticSampler(dx12_core, SamplerTypes::POINT_WRAP, 0)
		.AddConstant(dx12_core, ShaderVisibility::VERTEX, 0)
		.AddConstant(dx12_core, ShaderVisibility::PIXEL, 0)
		.AddConstant(dx12_core, ShaderVisibility::VERTEX, 1)
		.AddConstant(dx12_core, ShaderVisibility::VERTEX, 2)
		.AddConstant(dx12_core, ShaderVisibility::VERTEX, 1, 1)
		.AddConstant(dx12_core, ShaderVisibility::VERTEX, 3)
		.AddConstant(dx12_core, ShaderVisibility::VERTEX, 4)
		.InitRootSignature(dx12_core);

	m_pipeline.AddDepthStencil(false).InitPipeline(dx12_core, &m_root_signature, m_vertex_shader_path, m_pixel_shader_path);
}
