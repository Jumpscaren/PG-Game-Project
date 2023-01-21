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
}
