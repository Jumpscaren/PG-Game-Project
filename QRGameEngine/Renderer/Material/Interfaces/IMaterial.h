#pragma once
#include <string>
#include "Renderer/Material/MaterialTypes.h"

class IMaterial
{
public:
	virtual ~IMaterial() = default;

	virtual void SetVertexShader(const std::wstring& vertex_shader_path) = 0;
	virtual void SetPixelShader(const std::wstring& pixel_shader_path) = 0;

	virtual material_types::MaterialParameterIndex CreateFloatMaterialParameter(const std::string& material_parameter_name, const ShaderVisibility& shader_visibility, uint32_t shader_binding_index, uint32_t shader_space = 0) = 0;
	virtual void SetValueToMaterialParameter(material_types::MaterialParameterIndex material_parameter_index, float value) = 0;
	virtual material_types::MaterialParameterIndex GetMaterialParameterIndex(const std::string& material_parameter_name) const = 0;
};