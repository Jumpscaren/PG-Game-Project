#include "pch.h"
#include "CSMonoMethod.h"
#include "CSMonoCore.h"
#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/debug-helpers.h>

std::string CSMonoMethod::GetMethodFullName(CSMonoClass* mono_class, const std::string& method_name)
{
	return std::move(mono_class->GetMonoClassFullName() + "::" + method_name);
}

_MonoMethod* CSMonoMethod::GetMonoMethodFromMono(CSMonoClass* mono_class, const std::string& method_name)
{
	std::string full_name = GetMethodFullName(mono_class, method_name);
	MonoMethodDesc* desc = mono_method_desc_new(full_name.c_str(), false);
	return mono_method_desc_search_in_class(desc, mono_class->GetMonoClass());
}

CSMonoMethod::CSMonoMethod(CSMonoClass* mono_class, const MonoClassHandle& class_handle, const std::string& method_name) : m_class_handle(class_handle), m_method_name(method_name)
{
	m_mono_method = GetMonoMethodFromMono(mono_class, method_name);

	assert(m_mono_method);
}

bool CSMonoMethod::CheckIfMonoMethodExists(CSMonoClass* mono_class, const std::string& method_name)
{
	return GetMonoMethodFromMono(mono_class, method_name) != nullptr;
}

_MonoMethod* CSMonoMethod::GetMonoMethod() const
{
	return m_mono_method;
}
