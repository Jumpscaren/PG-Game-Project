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

_MonoClass* CSMonoClass::GetMonoClass() const
{
	return m_mono_class;
}

CSMonoClass::CSMonoClass(CSMonoCore* mono_core, const std::string& mono_namespace, const std::string& mono_class) : m_mono_namespace(mono_namespace), m_mono_class_name(mono_class)
{
	m_mono_class = mono_class_from_name(mono_core->GetImage(), mono_namespace.c_str(), mono_class.c_str());

	assert(m_mono_class);

	m_mono_full_name = m_mono_namespace + "." + m_mono_class_name;
}
