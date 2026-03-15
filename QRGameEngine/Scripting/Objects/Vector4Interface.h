#pragma once
#include "Scripting/CSMonoObject.h"
#include "Common/EngineTypes.h"

class Vector4Interface
{
private:
	static MonoClassHandle s_vector_4_class;
	static MonoFieldHandle s_x_field;
	static MonoFieldHandle s_y_field;
	static MonoFieldHandle s_z_field;
	static MonoFieldHandle s_w_field;

public:
	static void RegisterInterface(CSMonoCore* mono_core);

public:
	static CSMonoObject CreateVector4(const Vector4& vector_4);
	static Vector4 GetVector4(const CSMonoObject& cs_vector_4);
};

