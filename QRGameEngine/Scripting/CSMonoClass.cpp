#include "pch.h"
#include "CSMonoClass.h"
#include "CSMonoCore.h"
#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/debug-helpers.h>

const std::string& CSMonoClass::GetMonoClassName() const
{
	return m_mono_class_name;
}

const std::string& CSMonoClass::GetMonoClassFullName() const
{
	return m_mono_full_name;
}

std::string CSMonoClass::GetMonoClassFullName(const std::string& mono_namespace, const std::string& mono_class)
{
	return mono_namespace + "." + mono_class;
}

const std::string& CSMonoClass::GetMonoNamespace() const
{
	return m_mono_namespace;
}

_MonoClass* CSMonoClass::GetMonoClass() const
{
	return m_mono_class;
}

MonoFieldHandle CSMonoClass::AddField(const std::string& field_name)
{
	MonoFieldHandle field_handle = {};
	field_handle.handle = m_field_handles.size();

	MonoClassField* field = mono_class_get_field_from_name(m_mono_class, field_name.c_str());
	assert(field);

	uint32_t token = mono_class_get_field_token(field);
	m_field_handles.push_back(token);

	return field_handle;
}

uint32_t CSMonoClass::GetFieldToken(const MonoFieldHandle& field_handle)
{
	assert(field_handle.handle < m_field_handles.size());
	return m_field_handles[field_handle.handle];
}

CSMonoClass::CSMonoClass(CSMonoCore* mono_core, const std::string& mono_namespace, const std::string& mono_class) : m_mono_namespace(mono_namespace), m_mono_class_name(mono_class)
{
	m_mono_class = mono_class_from_name(mono_core->GetImage(), mono_namespace.c_str(), mono_class.c_str());

	assert(m_mono_class);

	m_mono_full_name = GetMonoClassFullName(mono_namespace, mono_class);
}
