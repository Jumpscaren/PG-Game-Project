#include "pch.h"
#include "CSMonoMethod.h"
#include "CSMonoCore.h"
#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/debug-helpers.h>

CSMonoMethod::CSMonoMethod(CSMonoClass* mono_class, const MonoClassHandle& class_handle, const std::string& method_name) : m_class_handle(class_handle), m_method_name(method_name)
{
	std::string full_name = mono_class->GetMonoClassFullName() + "::" + m_method_name;
	MonoMethodDesc* desc = mono_method_desc_new(full_name.c_str(), false);
	m_mono_method = mono_method_desc_search_in_class(desc, mono_class->GetMonoClass());

	assert(m_mono_method);
}

_MonoMethod* CSMonoMethod::GetMonoMethod() const
{
	return m_mono_method;
}
