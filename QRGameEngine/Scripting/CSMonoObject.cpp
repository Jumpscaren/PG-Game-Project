#include "pch.h"
#include "CSMonoObject.h"
#include "CSMonoCore.h"
#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/debug-helpers.h>

_MonoObject* CSMonoObject::GetMonoObject() const
{
	assert(!m_not_initialized);
	assert(mono_gchandle_get_target(m_gchandle) != nullptr);
	return mono_gchandle_get_target(m_gchandle);
}

void CSMonoObject::CreateLinkToMono(_MonoObject* mono_object)
{
	m_gchandle = mono_gchandle_new(mono_object, false);
	//std::cout << "New handle: " << m_gchandle << "\n";
}

CSMonoObject::CSMonoObject(CSMonoCore* mono_core, const MonoClassHandle& class_handle) : m_class_handle(class_handle), m_mono_core_ref(mono_core)
{
	MonoObject* mono_object = mono_object_new(mono_core->GetDomain(), mono_core->GetMonoClass(class_handle)->GetMonoClass());
	CreateLinkToMono(mono_object);
	mono_runtime_object_init(mono_object);
}

CSMonoObject::CSMonoObject(CSMonoCore* mono_core, _MonoObject* mono_object) : m_mono_core_ref(mono_core)
{
	CreateLinkToMono(mono_object);
	MonoClass* mono_class = mono_object_get_class(mono_object);
	m_class_handle = mono_core->RegisterMonoClass(mono_class);
}

CSMonoObject::CSMonoObject(const CSMonoObject& obj) : m_class_handle(obj.m_class_handle), m_mono_core_ref(obj.m_mono_core_ref), m_not_initialized(obj.m_not_initialized)
{
	CreateLinkToMono(obj.GetMonoObject());
}

CSMonoObject::CSMonoObject(CSMonoObject&& obj) noexcept : m_class_handle(obj.m_class_handle), m_mono_core_ref(obj.m_mono_core_ref), m_not_initialized(obj.m_not_initialized)
{
	CreateLinkToMono(obj.GetMonoObject());
}

CSMonoObject::CSMonoObject()
{
	m_class_handle.handle = -1;
	m_mono_core_ref = nullptr;
	m_gchandle = 0;
	m_not_initialized = true;
}

CSMonoObject::~CSMonoObject()
{
	RemoveLinkToMono();
}

CSMonoObject& CSMonoObject::operator=(const CSMonoObject& obj)
{
	CreateLinkToMono(obj.GetMonoObject());
	m_class_handle = obj.m_class_handle;
	m_mono_core_ref = obj.m_mono_core_ref;
	m_not_initialized = obj.m_not_initialized;
	return *this;
}

CSMonoObject& CSMonoObject::operator=(CSMonoObject&& obj) noexcept
{
	CreateLinkToMono(obj.GetMonoObject());
	m_class_handle = obj.m_class_handle;
	m_mono_core_ref = obj.m_mono_core_ref;
	m_not_initialized = obj.m_not_initialized;
	return *this;
}

void CSMonoObject::RemoveLinkToMono()
{
	//std::cout << "Free handle: " << m_gchandle << "\n";
	m_not_initialized = false;
	mono_gchandle_free(m_gchandle);
	m_gchandle = -1;
}
