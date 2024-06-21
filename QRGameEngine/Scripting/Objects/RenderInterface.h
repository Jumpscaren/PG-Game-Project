#pragma once
#include "Scripting/CSMonoCore.h"

class RenderInterface
{
private:
	static MonoClassHandle texture_handle;

public:
	static void RegisterInterface(CSMonoCore* mono_core);

public:
	static CSMonoObject LoadTexture(const std::string& texture_name);
};

