#pragma once
#include "Scripting/CSMonoCore.h"

class MaterialInterface
{
public:
	static void RegisterInterface(CSMonoCore* mono_core);

public:
	static uint32_t GetMaterialParameter(uint32_t material_index, const std::string& material_parameter_name);
};
