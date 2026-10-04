#include "pch.h"
#include "MaterialDatabase.h"

MaterialIndex MaterialDatabase::CreateMaterial(const std::string& material_name)
{
	MaterialIndex materialIndex{.index = static_cast<uint32_t>(m_materials.size())};
	m_materials.push_back(Material());
	m_material_name_to_index.emplace(material_name, materialIndex);
	return materialIndex;
}

MaterialIndex MaterialDatabase::GetMaterialIndex(const std::string& material_name)
{
	assert(m_material_name_to_index.contains(material_name));
	return m_material_name_to_index.at(material_name);
}

IMaterial* MaterialDatabase::GetIMaterial(const MaterialIndex materialIndex)
{
	assert(m_materials.size() > materialIndex.index);
	return &m_materials[materialIndex.index];
}

Material* MaterialDatabase::GetMaterial(const MaterialIndex materialIndex)
{
	assert(m_materials.size() > materialIndex.index);
	return &m_materials[materialIndex.index];
}

void MaterialDatabase::Initialize(const MaterialIndex materialIndex)
{
	m_materials[materialIndex.index].Initialize(m_dx12_core);
}
