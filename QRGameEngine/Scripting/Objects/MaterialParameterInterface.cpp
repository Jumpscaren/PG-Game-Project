#include "pch.h"
#include "MaterialParameterInterface.h"
#include "Renderer/RenderCore.h"

void MaterialParameterInterface::RegisterInterface(CSMonoCore* mono_core)
{
	const MonoClassHandle material_parameter_handle = mono_core->RegisterMonoClass("ScriptProject.Engine.Types", "MaterialParameter");

	mono_core->HookAndRegisterMonoMethodType<MaterialParameterInterface::SetFloat>(material_parameter_handle, "SetFloat_External", MaterialParameterInterface::SetFloat);
}

void MaterialParameterInterface::SetFloat(const uint32_t material_index, const uint32_t material_parameter_index, const float value)
{
	IMaterial* material = RenderCore::Get()->GetMaterialDatabase()->GetIMaterial(material_types::MaterialIndex{ .index = material_index });
	material->SetValueToMaterialParameter(material_types::MaterialParameterIndex{ .index = material_parameter_index }, value);
}
