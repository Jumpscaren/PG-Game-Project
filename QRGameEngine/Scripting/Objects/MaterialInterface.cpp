#include "pch.h"
#include "MaterialInterface.h"
#include "Renderer/RenderCore.h"

void MaterialInterface::RegisterInterface(CSMonoCore* mono_core)
{
	const MonoClassHandle material_handle = mono_core->RegisterMonoClass("ScriptProject.Engine.Types", "Material");

	mono_core->HookAndRegisterMonoMethodType<MaterialInterface::GetMaterialParameter>(material_handle, "GetMaterialParameter_External", MaterialInterface::GetMaterialParameter);
}

uint32_t MaterialInterface::GetMaterialParameter(const uint32_t material_index, const std::string& material_parameter_name)
{
	IMaterial* material = RenderCore::Get()->GetMaterialDatabase()->GetIMaterial(material_types::MaterialIndex{ .index = material_index });
	return static_cast<uint32_t>(material->GetMaterialParameterIndex(material_parameter_name).index);
}
