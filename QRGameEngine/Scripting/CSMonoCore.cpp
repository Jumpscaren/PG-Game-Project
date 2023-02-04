#include "pch.h"
#include "CSMonoCore.h"
#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/debug-helpers.h>

CSMonoClass* CSMonoCore::GetMonoClass(const MonoClassHandle& class_handle)
{
	return &m_mono_classes[class_handle.handle];
}

CSMonoMethod* CSMonoCore::GetMonoMethod(const MonoMethodHandle& method_handle)
{
	return &m_mono_methods[method_handle.handle];
}

_MonoDomain* CSMonoCore::GetDomain() const
{
	return m_domain;
}

CSMonoCore::CSMonoCore()
{
	mono_set_dirs("../QRGameEngine/", "../QRGameEngine/");

	m_domain = mono_jit_init("MonoCore");

	assert(m_domain);

	m_assembly = mono_domain_assembly_open(m_domain, "../ScriptProject/build/ScriptProject.dll");

	assert(m_assembly);

	m_image = mono_assembly_get_image(m_assembly);

	assert(m_image);
}

MonoImage* CSMonoCore::GetImage() const
{
	return m_image;
}

void CSMonoCore::HandleException(_MonoObject* exception)
{
	if (!exception)
		return;

	mono_print_unhandled_exception(exception);
}

void* CSMonoCore::ToMethodParameter(int& number)
{
	return &number;
}

void* CSMonoCore::ToMethodParameter(float& number)
{
	return &number;
}

void* CSMonoCore::ToMethodParameter(double& number)
{
	return &number;
}

void* CSMonoCore::ToMethodParameter(bool& boolean)
{
	return &boolean;
}

void* CSMonoCore::ToMethodParameter(const char* string)
{
	return mono_string_new(m_domain, string);
}

void* CSMonoCore::ToMethodParameter(const std::string& string)
{
	return mono_string_new(m_domain, string.c_str());
}

void* CSMonoCore::ToMethodParameter(CSMonoObject* mono_object)
{
	return mono_object->GetMonoObject();
}

void CSMonoCore::CallMethod(const MonoMethodHandle& method_handle, CSMonoObject* mono_object, void** parameters)
{
	MonoObject* exception = nullptr;

	mono_runtime_invoke(GetMonoMethod(method_handle)->GetMonoMethod(), mono_object, parameters, &exception);

	HandleException(exception);
}

MonoClassHandle CSMonoCore::RegisterMonoClass(const std::string& class_namespace, const std::string& class_name)
{
	MonoClassHandle class_handle = { m_mono_classes.size() };

	m_mono_classes.push_back(CSMonoClass(this, class_namespace, class_name));

	return class_handle;
}

MonoMethodHandle CSMonoCore::RegisterMonoMethod(const MonoClassHandle& class_handle, const std::string& method_name)
{
	MonoMethodHandle method_handle = { m_mono_methods.size() };

	m_mono_methods.push_back(CSMonoMethod(GetMonoClass(class_handle), class_handle, method_name));

	return method_handle;
}

void CSMonoCore::CallMethod(const MonoMethodHandle& method_handle)
{
	MonoObject* exception = nullptr;

	mono_runtime_invoke(GetMonoMethod(method_handle)->GetMonoMethod(), nullptr, nullptr, &exception);

	HandleException(exception);
}

void CSMonoCore::CallMethod(CSMonoObject* mono_object, const MonoMethodHandle& method_handle)
{
	MonoObject* exception = nullptr;

	mono_runtime_invoke(GetMonoMethod(method_handle)->GetMonoMethod(), mono_object->GetMonoObject(), nullptr, &exception);

	HandleException(exception);
}

void CSMonoCore::HookMethod(const MonoClassHandle& class_handle, const std::string& method_name, const void* method)
{
	CSMonoClass* mono_class = GetMonoClass(class_handle);
	std::string full_name =  mono_class->GetMonoClassFullName() + "::" + method_name;
	mono_add_internal_call(full_name.c_str(), method);
}

MonoMethodHandle CSMonoCore::HookAndRegisterMonoMethod(const MonoClassHandle& class_handle, const std::string& method_name, const void* method)
{
	HookMethod(class_handle, method_name, method);
	return RegisterMonoMethod(class_handle, method_name);
}
