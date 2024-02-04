#pragma once
#include "Scripting/CSMonoObject.h"
#include "SceneSystem/SceneDefines.h"
#include "ECS/EntityDefinition.h"
#include "Common/EngineTypes.h"

class Vector2Interface
{
private:
	static MonoClassHandle s_vector_2_class;
	static MonoFieldHandle s_x_field;
	static MonoFieldHandle s_y_field;

public:
	static void RegisterInterface(CSMonoCore* mono_core);

public:
	static CSMonoObject CreateVector2(const Vector2& vector_2);
	static Vector2 GetVector2(const CSMonoObject& cs_vector_2);
};

