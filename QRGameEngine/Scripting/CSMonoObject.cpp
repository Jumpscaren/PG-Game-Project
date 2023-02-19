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

CSMonoObject::CSMonoObject(CSMonoCore* mono_core, const MonoClassHandle& class_handle) : m_class_handle(class_handle), m_mono_core_ref(mono_core)
{
	m_mono_object = mono_object_new(mono_core->GetDomain(), mono_core->GetMonoClass(class_handle)->GetMonoClass());
	m_gchandle = mono_gchandle_new(m_mono_object, false);
	mono_runtime_object_init(m_mono_object);
}

CSMonoObject::CSMonoObject(CSMonoCore* mono_core, _MonoObject* mono_object) : m_mono_core_ref(mono_core), m_mono_object(mono_object)
{
	m_gchandle = mono_gchandle_new(m_mono_object, false);
	MonoClass* mono_class = mono_object_get_class(m_mono_object);
	m_class_handle = mono_core->RegisterMonoClass(mono_class);
}

CSMonoObject::CSMonoObject(const CSMonoObject& obj) : m_mono_object(obj.m_mono_object), m_class_handle(obj.m_class_handle), m_mono_core_ref(obj.m_mono_core_ref)
{
	m_gchandle = mono_gchandle_new(m_mono_object, false);
}

CSMonoObject::~CSMonoObject()
{
	mono_gchandle_free(m_gchandle);
}

void CSMonoObject::CallMethod(const MonoMethodHandle& method_handle)
{
	m_mono_core_ref->CallMethod(method_handle, this);
}
