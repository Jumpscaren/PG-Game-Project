#include "pch.h"
#include "Scripting/CSMonoCore.h"
#include "Vector4Interface.h"

MonoClassHandle Vector4Interface::s_vector_4_class;
MonoFieldHandle Vector4Interface::s_x_field;
MonoFieldHandle Vector4Interface::s_y_field;
MonoFieldHandle Vector4Interface::s_z_field;
MonoFieldHandle Vector4Interface::s_w_field;

void Vector4Interface::RegisterInterface(CSMonoCore* mono_core)
{
    s_vector_4_class = mono_core->RegisterMonoClass("ScriptProject.EngineMath", "Vector4");
    s_x_field = mono_core->RegisterField(s_vector_4_class, "x");
    s_y_field = mono_core->RegisterField(s_vector_4_class, "y");
    s_z_field = mono_core->RegisterField(s_vector_4_class, "z");
    s_w_field = mono_core->RegisterField(s_vector_4_class, "w");
}

CSMonoObject Vector4Interface::CreateVector4(const Vector4& vector_4)
{
    CSMonoObject cs_vector_4(CSMonoCore::Get(), s_vector_4_class);
    CSMonoCore::Get()->SetValue(vector_4.x, cs_vector_4, s_x_field);
    CSMonoCore::Get()->SetValue(vector_4.y, cs_vector_4, s_y_field);
    CSMonoCore::Get()->SetValue(vector_4.z, cs_vector_4, s_z_field);
    CSMonoCore::Get()->SetValue(vector_4.w, cs_vector_4, s_w_field);

    return cs_vector_4;
}

Vector4 Vector4Interface::GetVector4(const CSMonoObject& cs_vector_4)
{
    Vector4 vector_4;
    CSMonoCore::Get()->GetValue(vector_4.x, cs_vector_4, s_x_field);
    CSMonoCore::Get()->GetValue(vector_4.y, cs_vector_4, s_y_field);
    CSMonoCore::Get()->GetValue(vector_4.z, cs_vector_4, s_z_field);
    CSMonoCore::Get()->GetValue(vector_4.w, cs_vector_4, s_w_field);

    return vector_4;
}
