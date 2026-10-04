#pragma once
#include "Renderer/Material/Interfaces/IMaterialDatabase.h"

class IRenderCore
{
public:
	static IMaterialDatabase* GetMaterialDatabase();
};