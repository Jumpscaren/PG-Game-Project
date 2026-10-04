#include "pch.h"
#include "Renderer/RenderCore.h"
#include "IRenderCore.h"

IMaterialDatabase* IRenderCore::GetMaterialDatabase() {
	return RenderCore::Get()->GetMaterialDatabase();
}

