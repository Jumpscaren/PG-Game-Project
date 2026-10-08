#pragma once
#include "Scripting/CSMonoCore.h"

class MaterialParameterInterface
{
public:
	static void RegisterInterface(CSMonoCore* mono_core);

public:
	static void SetFloat(uint32_t material_index, uint32_t material_parameter_index, float value);
};
