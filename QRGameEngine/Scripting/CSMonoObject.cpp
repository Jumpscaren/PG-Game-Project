#include "pch.h"
#include "CSMonoObject.h"
#include "CSMonoCore.h"
#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/debug-helpers.h>

std::unordered_set<uint32_t> CSMonoObject::test;

_MonoObject* CSMonoObject::GetMonoObject() const
{
	assert(!m_not_initialized);
	_MonoObject* const mono_object = mono_gchandle_get_target(m_gchandle);
	assert(mono_object != nullptr);
	return mono_object;
}

void CSMonoObject::CreateLinkToMono(_MonoObject* mono_object)
{
	if (m_gchandle != -1)
	{
		assert(false);
	}
	m_gchandle = mono_gchandle_new(mono_object, false);
	//test.emplace(m_gchandle);
	
	//std::cout << "New handle: " << m_gchandle << "\n";
}

CSMonoObject::CSMonoObject(CSMonoCore* mono_core, const MonoClassHandle class_handle) : m_mono_core_ref(mono_core)
{
	m_mono_class = mono_core->GetMonoClass(class_handle)->GetMonoClass();
	MonoObject* mono_object = mono_object_new(mono_core->GetDomain(), m_mono_class);
	mono_runtime_object_init(mono_object);
	CreateLinkToMono(mono_object);
}

CSMonoObject::CSMonoObject(CSMonoCore* mono_core, _MonoObject* mono_object) : m_mono_core_ref(mono_core)
{
	m_mono_class = mono_object_get_class(mono_object);
	//m_class_handle = mono_core->RegisterMonoClass(mono_class);
	CreateLinkToMono(mono_object);
}

CSMonoObject::CSMonoObject(const CSMonoObject& obj) : m_mono_class(obj.m_mono_class), m_mono_core_ref(obj.m_mono_core_ref), m_not_initialized(obj.m_not_initialized)
{
	CreateLinkToMono(obj.GetMonoObject());
}

CSMonoObject::CSMonoObject(CSMonoObject&& obj) noexcept : m_mono_class(obj.m_mono_class), m_mono_core_ref(obj.m_mono_core_ref), m_not_initialized(obj.m_not_initialized)
{
	CreateLinkToMono(obj.GetMonoObject());
}

CSMonoObject::CSMonoObject()
{
	m_mono_class = nullptr;
	m_mono_core_ref = nullptr;
	m_gchandle = -1;
	m_not_initialized = true;
}

CSMonoObject::~CSMonoObject()
{
	RemoveLinkToMono();
}

CSMonoObject& CSMonoObject::operator=(const CSMonoObject& obj)
{
	m_mono_class = obj.m_mono_class;
	m_mono_core_ref = obj.m_mono_core_ref;
	m_not_initialized = obj.m_not_initialized;
	CreateLinkToMono(obj.GetMonoObject());
	return *this;
}

CSMonoObject& CSMonoObject::operator=(CSMonoObject&& obj) noexcept
{
	m_mono_class = obj.m_mono_class;
	m_mono_core_ref = obj.m_mono_core_ref;
	m_not_initialized = obj.m_not_initialized;
	CreateLinkToMono(obj.GetMonoObject());
	return *this;
}

void CSMonoObject::RemoveLinkToMono()
{
	//std::cout << "Free handle: " << m_gchandle << "\n";
	if (m_gchandle != -1)
	{
		m_not_initialized = true;
		mono_gchandle_free(m_gchandle);
		//if (test.find(m_gchandle) != test.end())
		//{
		//	test.erase(m_gchandle);
		//}
		m_mono_class = nullptr;
		m_gchandle = -1;
	}
}

std::string CSMonoObject::GetName(const uint32_t gchandle)
{
	return mono_class_get_name(mono_object_get_class(mono_gchandle_get_target(gchandle)));
}
