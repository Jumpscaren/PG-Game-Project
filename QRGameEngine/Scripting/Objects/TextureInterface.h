#pragma once
#include "Scripting/CSMonoHandles.h"
#include "ECS/EntityDefinition.h"
#include "SceneSystem/SceneDefines.h"
#include "Renderer/RenderTypes.h"
#include "Scripting/CSMonoObject.h"

class CSMonoCore;

class TextureInterface
{
private:
	static MonoClassHandle s_texture_object_class;

public:
	static void RegisterInterface(CSMonoCore* mono_core);

public:
	static CSMonoObject CreateTexture(TextureHandle texture_handle);
};

