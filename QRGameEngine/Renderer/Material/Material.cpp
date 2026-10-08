#include "pch.h"
#include "Material.h"
#include "Renderer/DX12CORE/DX12Core.h"

Material::Material()
{
	m_vertex_buffer_material_index = CreateMaterialParameter<DX12BufferViewHandle>("VerticesBuffer", ShaderVisibility::VERTEX, 0);
	m_transform_buffer_material_index = CreateMaterialParameter<DX12BufferViewHandle>("TransformBuffer", ShaderVisibility::VERTEX, 1);
	m_vertex_sprite_buffer_material_index = CreateMaterialParameter<DX12BufferViewHandle>("VertexSpriteBuffer", ShaderVisibility::VERTEX, 1, 1);
	m_camera_buffer_material_index = CreateMaterialParameter<DX12BufferViewHandle>("CameraBuffer", ShaderVisibility::VERTEX, 2);
	m_indicies_buffer_material_index = CreateMaterialParameter<DX12BufferViewHandle>("IndiciesBuffer", ShaderVisibility::VERTEX, 3);
	m_start_index_material_index = CreateMaterialParameter<uint32_t>("StartIndex", ShaderVisibility::VERTEX, 4);

	m_pixel_sprite_buffer_material_index = CreateMaterialParameter<DX12BufferViewHandle>("PixelSpriteBuffer", ShaderVisibility::PIXEL, 0);
}

void Material::Initialize(DX12Core* dx12_core)
{
	m_root_signature.AddStaticSampler(dx12_core, SamplerTypes::POINT_WRAP, 0);
	for (material_types::MaterialParameterData& material_parameter : m_material_parameters)
	{
		//Depends on the data type but for right now, everything is just a constant
		m_root_signature.AddConstant(dx12_core, material_parameter.root_parameter_index, material_parameter.shader_visibility, material_parameter.shader_binding_index, material_parameter.shader_space);
	}
	m_root_signature.InitRootSignature(dx12_core);

	m_pipeline.AddDepthStencil(false).InitPipeline(dx12_core, &m_root_signature, m_vertex_shader_path, m_pixel_shader_path);
}
