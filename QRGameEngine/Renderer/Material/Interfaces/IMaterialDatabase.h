#pragma once
#include "Renderer/Material/MaterialTypes.h"
#include "IMaterial.h"

class IMaterialDatabase
{
public:
	virtual ~IMaterialDatabase() = default;

	virtual material_types::MaterialIndex CreateMaterial(const std::string& material_name) = 0;
	virtual material_types::MaterialIndex GetMaterialIndex(const std::string& material_name) = 0;
	virtual IMaterial* GetIMaterial(material_types::MaterialIndex materialIndex) = 0;
	virtual void Initialize(material_types::MaterialIndex materialIndex) = 0;
};