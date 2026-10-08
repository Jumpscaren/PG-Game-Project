#pragma once
#include "Material.h"
#include "MaterialTypes.h"
#include "Interfaces/IMaterialDatabase.h"

class DX12Core;

class MaterialDatabase : public IMaterialDatabase
{
public:
	~MaterialDatabase() override = default;

	void SetDX12Core(DX12Core* dx12_core) { m_dx12_core = dx12_core; }
	material_types::MaterialIndex CreateMaterial(const std::string& material_name) override;
	material_types::MaterialIndex GetMaterialIndex(const std::string& material_name) override;
	IMaterial* GetIMaterial(material_types::MaterialIndex materialIndex) override;
	Material* GetMaterial(material_types::MaterialIndex materialIndex);
	void Initialize(material_types::MaterialIndex materialIndex) override;

private:
	std::vector<Material> m_materials;
	qr::unordered_map<std::string, material_types::MaterialIndex> m_material_name_to_index;

	DX12Core* m_dx12_core = nullptr;
};

