#pragma once
#include "Renderer/DX12CORE/DX12RootSignature.h"
#include "Renderer/DX12CORE/DX12Pipeline.h"
#include "Interfaces/IMaterial.h"
#include "MaterialTypes.h"

class DX12Core;

class Material : public IMaterial
{
	friend class MaterialDatabase;
public:
	Material();
	~Material() override = default;

	void SetVertexShader(const std::wstring& vertex_shader_path) override { m_vertex_shader_path = vertex_shader_path; }
	void SetPixelShader(const std::wstring& pixel_shader_path) override { m_pixel_shader_path = pixel_shader_path; }

	material_types::MaterialParameterIndex CreateFloatMaterialParameter(const std::string& material_parameter_name, const ShaderVisibility& shader_visibility, uint32_t shader_binding_index, uint32_t shader_space = 0) override { return CreateMaterialParameter<float>(material_parameter_name, shader_visibility, shader_binding_index, shader_space); }
	void SetValueToMaterialParameter(material_types::MaterialParameterIndex material_parameter_index, float value) override { SetValueToMaterialParameter<float>(material_parameter_index, value); }

	DX12RootSignature* GetRootSignature() { return &m_root_signature; }
	DX12Pipeline* GetPipeline() { return &m_pipeline; }

	template <material_types::IsAllowedMaterialParameterType T>
	material_types::MaterialParameterIndex CreateMaterialParameter(const std::string& material_parameter_name, const ShaderVisibility& shader_visibility, uint32_t shader_binding_index, uint32_t shader_space = 0);
	template <material_types::IsAllowedMaterialParameterType T>
	void SetValueToMaterialParameter(material_types::MaterialParameterIndex material_parameter_index, const T& value);
	material_types::MaterialParameterIndex GetMaterialParameterIndex(const std::string& material_parameter_name) const override { assert(m_material_name_to_index.contains(material_parameter_name)); return m_material_name_to_index.at(material_parameter_name); }

	material_types::MaterialParameterIndex GetVertexBufferMaterialIndex() const { return m_vertex_buffer_material_index; }
	material_types::MaterialParameterIndex GetTransformBufferMaterialIndex() const { return m_transform_buffer_material_index; }
	material_types::MaterialParameterIndex GetVertexSpriteBufferMaterialIndex() const { return m_vertex_sprite_buffer_material_index; }
	material_types::MaterialParameterIndex GetCameraBufferMaterialIndex() const { return m_camera_buffer_material_index; }
	material_types::MaterialParameterIndex GetIndiciesBufferMaterialIndex() const { return m_indicies_buffer_material_index; }
	material_types::MaterialParameterIndex GetStartIndexMaterialIndex() const { return m_start_index_material_index; }

	material_types::MaterialParameterIndex GetPixelSpriteBufferMaterialIndex() const { return m_pixel_sprite_buffer_material_index; }

	const std::vector<material_types::MaterialParameterData>& GetMaterialParameters() const { return m_material_parameters; }

private:
	DX12RootSignature m_root_signature;
	DX12Pipeline m_pipeline;

	std::wstring m_vertex_shader_path = L"../QRGameEngine/Shaders/VertexShader.hlsl";
	std::wstring m_pixel_shader_path = L"../QRGameEngine/Shaders/PixelShader.hlsl";

	qr::unordered_map<std::string, material_types::MaterialParameterIndex> m_material_name_to_index;
	std::vector<material_types::MaterialParameterData> m_material_parameters;
	material_types::MaterialParameterIndex m_current_parameter_index{};

	//Main Pipeline Parameters
	material_types::MaterialParameterIndex m_vertex_buffer_material_index{};
	material_types::MaterialParameterIndex m_transform_buffer_material_index{};
	material_types::MaterialParameterIndex m_vertex_sprite_buffer_material_index{};
	material_types::MaterialParameterIndex m_camera_buffer_material_index{};
	material_types::MaterialParameterIndex m_indicies_buffer_material_index{};
	material_types::MaterialParameterIndex m_start_index_material_index{};

	material_types::MaterialParameterIndex m_pixel_sprite_buffer_material_index{};

private:
	void Initialize(DX12Core* dx12_core);
};

template<material_types::IsAllowedMaterialParameterType T>
inline material_types::MaterialParameterIndex Material::CreateMaterialParameter(const std::string& material_parameter_name, const ShaderVisibility& shader_visibility, const uint32_t shader_binding_index, const uint32_t shader_space)
{
	const material_types::MaterialParameterIndex new_material_parameter_index = m_current_parameter_index;
	++m_current_parameter_index.index;
	m_material_parameters.push_back(material_types::MaterialParameterData{ .root_parameter_index = {}, .value = T{}, .shader_visibility = shader_visibility, .shader_binding_index = shader_binding_index, .shader_space = shader_space });

	m_material_name_to_index.emplace(material_parameter_name, new_material_parameter_index);

	return new_material_parameter_index;
}

template<material_types::IsAllowedMaterialParameterType T>
inline void Material::SetValueToMaterialParameter(const material_types::MaterialParameterIndex material_parameter_index, const T& value)
{
	assert(material_parameter_index < m_current_parameter_index);

	material_types::MaterialParameterData& material_parameter_data = m_material_parameters.at(material_parameter_index.index);
	if (std::holds_alternative<T>(material_parameter_data.value))
	{
		material_parameter_data.value = value;
		return;
	}

	assert(false);
	std::cout << "SetValueToMaterialParameter: Invalid Material Parameter Type" << std::endl;
}
