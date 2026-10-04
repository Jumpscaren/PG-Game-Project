#pragma once
#include "Renderer/Material/MaterialTypes.h"
#include "IMaterial.h"

class IMaterialDatabase
{
public:
	virtual ~IMaterialDatabase() = default;

	virtual MaterialIndex CreateMaterial(const std::string& material_name) = 0;
	virtual MaterialIndex GetMaterialIndex(const std::string& material_name) = 0;
	virtual IMaterial* GetIMaterial(MaterialIndex materialIndex) = 0;
	virtual void Initialize(MaterialIndex materialIndex) = 0;
};