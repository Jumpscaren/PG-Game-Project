#include "MaterialCreator.h"
#include "Renderer/Material/Interfaces/IMaterial.h"
#include "Renderer/Interfaces/IRenderCore.h"
#include "Renderer/Material/MaterialTypes.h"

void MaterialCreator::CreateMaterials()
{
	IMaterialDatabase* material_database = IRenderCore::GetMaterialDatabase();

	material_types::MaterialIndex material_index = material_database->CreateMaterial("SlashAttackMaterial");
	IMaterial* material = material_database->GetIMaterial(material_index);
	material->SetPixelShader(L"Shaders/SlashAttackPixelShader.hlsl");
	material->CreateFloatMaterialParameter("radius", ShaderVisibility::PIXEL, 1);

	material_database->Initialize(material_index);
}
