#include "pch.h"
#include "CSMonoObject.h"
#include "CSMonoCore.h"
#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/debug-helpers.h>

_MonoObject* CSMonoObject::GetMonoObject() const
{
	return m_mono_object;
}

CSMonoObject::CSMonoObject(CSMonoCore* mono_core, const MonoClassHandle& class_handle) : m_class_handle(class_handle)
{
	m_mono_object = mono_object_new(mono_core->GetDomain(), mono_core->GetMonoClass(class_handle)->GetMonoClass());
	m_gchandle = mono_gchandle_new(m_mono_object, false);
	mono_runtime_object_init(m_mono_object);
}

CSMonoObject::~CSMonoObject()
{
	mono_gchandle_free(m_gchandle);
}
