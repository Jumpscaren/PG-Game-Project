#include "pch.h"
#include "Vector2Interface.h"
#include "Scripting/CSMonoCore.h"

MonoClassHandle Vector2Interface::s_vector_2_class;
MonoFieldHandle Vector2Interface::s_x_field;
MonoFieldHandle Vector2Interface::s_y_field;

void Vector2Interface::RegisterInterface(CSMonoCore* mono_core)
{
    s_vector_2_class = mono_core->RegisterMonoClass("ScriptProject.EngineMath", "Vector2");
    s_x_field = mono_core->RegisterField(s_vector_2_class, "x");
    s_y_field = mono_core->RegisterField(s_vector_2_class, "y");
}

CSMonoObject Vector2Interface::CreateVector2(const Vector2& vector_2)
{
    CSMonoObject cs_vector_2(CSMonoCore::Get(), s_vector_2_class);
    CSMonoCore::Get()->SetValue(vector_2.x, cs_vector_2, s_x_field);
    CSMonoCore::Get()->SetValue(vector_2.y, cs_vector_2, s_y_field);

    return cs_vector_2;
}

Vector2 Vector2Interface::GetVector2(const CSMonoObject& cs_vector_2)
{
    Vector2 vector_2;
    CSMonoCore::Get()->GetValue(vector_2.x, cs_vector_2, s_x_field);
    CSMonoCore::Get()->GetValue(vector_2.y, cs_vector_2, s_y_field);
    return vector_2;
}
