#pragma once
#include "Scripting/CSMonoCore.h"
#include "SceneSystem/SceneDefines.h"
#include "Common/DeferMethodCallsDefine.h"

class RenderInterface
{
private:
	static MonoClassHandle texture_handle;
	static DeferedMethodIndex s_load_texture_index;

public:
	static void RegisterInterface(CSMonoCore* mono_core, const DeferedMethodIndex load_texture_index);

public:
	static CSMonoObject LoadTexture(const std::string& texture_name, SceneIndex scene_index);
	static void LoadAndSetTexture(const CSMonoObject& texture, const std::string& texture_name, SceneIndex scene_index);
};

