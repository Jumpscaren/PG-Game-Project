#include "pch.h"
#include "CSMonoCore.h"
#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/debug-helpers.h>

CSMonoCore* CSMonoCore::s_mono_core = nullptr;

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

	s_mono_core = this;
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

void* CSMonoCore::ToMethodParameter(uint32_t& number)
{
	return &number;
}

void* CSMonoCore::ToMethodParameter(uint64_t& number)
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

void* CSMonoCore::ToMethodParameter(const CSMonoObject& mono_object)
{
	return mono_object.GetMonoObject();
}

int CSMonoCore::MonoObjectToValue(int* mono_object)
{
	return *(int*)(mono_object_unbox((MonoObject*)mono_object));
}

uint32_t CSMonoCore::MonoObjectToValue(uint32_t* mono_object)
{
	return *(uint32_t*)(mono_object_unbox((MonoObject*)mono_object));
}

uint64_t CSMonoCore::MonoObjectToValue(uint64_t* mono_object)
{
	return *(uint64_t*)(mono_object_unbox((MonoObject*)mono_object));
}

float CSMonoCore::MonoObjectToValue(float* mono_object)
{
	return *(float*)(mono_object_unbox((MonoObject*)mono_object));
}

double CSMonoCore::MonoObjectToValue(double* mono_object)
{
	return *(double*)(mono_object_unbox((MonoObject*)mono_object));
}

bool CSMonoCore::MonoObjectToValue(bool* mono_object)
{
	return *(bool*)(mono_object_unbox((MonoObject*)mono_object));
}

std::string CSMonoCore::MonoObjectToValue(std::string* mono_object)
{
	MonoString* mono_string = mono_object_to_string((MonoObject*)mono_object, nullptr);
	std::string return_string = mono_string_to_utf8(mono_string);
	return std::move(return_string);
}

CSMonoObject CSMonoCore::MonoObjectToValue(CSMonoObject* mono_object)
{
	return std::move(CSMonoObject(this, (MonoObject*)(mono_object)));
}

int CSMonoCore::MonoMethodParameter(int mono_parameter)
{
	return mono_parameter;
}

uint32_t CSMonoCore::MonoMethodParameter(uint32_t mono_parameter)
{
	return mono_parameter;
}

uint64_t CSMonoCore::MonoMethodParameter(uint64_t mono_parameter)
{
	return mono_parameter;
}

float CSMonoCore::MonoMethodParameter(float mono_parameter)
{
	return mono_parameter;
}

double CSMonoCore::MonoMethodParameter(double mono_parameter)
{
	return mono_parameter;
}

bool CSMonoCore::MonoMethodParameter(bool mono_parameter)
{
	return mono_parameter;
}

std::string CSMonoCore::MonoMethodParameter(_MonoString* mono_parameter)
{
	std::string string = mono_string_to_utf8(mono_parameter);
	return std::move(string);
}

CSMonoObject CSMonoCore::MonoMethodParameter(_MonoObject* mono_parameter)
{
	return std::move(CSMonoObject(s_mono_core, (MonoObject*)(mono_parameter)));
}

int CSMonoCore::MonoMethodReturn(int mono_return)
{
	return mono_return;
}

uint32_t CSMonoCore::MonoMethodReturn(uint32_t mono_return)
{
	return mono_return;
}

uint64_t CSMonoCore::MonoMethodReturn(uint64_t mono_return)
{
	return mono_return;
}

float CSMonoCore::MonoMethodReturn(float mono_return)
{
	return mono_return;
}

double CSMonoCore::MonoMethodReturn(double mono_return)
{
	return mono_return;
}

bool CSMonoCore::MonoMethodReturn(bool mono_return)
{
	return mono_return;
}

_MonoString* CSMonoCore::MonoMethodReturn(const std::string& mono_return)
{
	return mono_string_new(s_mono_core->m_domain, mono_return.c_str());
}

_MonoObject* CSMonoCore::MonoMethodReturn(const CSMonoObject& mono_return)
{
	return mono_return.GetMonoObject();
}

_MonoObject* CSMonoCore::CallMethodInternal(const MonoMethodHandle& method_handle, _MonoObject* mono_object, void** parameters, uint32_t parameter_count)
{
	MonoObject* exception = nullptr;

	MonoMethod* method = GetMonoMethod(method_handle)->GetMonoMethod();
	
#ifdef _DEBUG
	uint32_t param_amount = mono_signature_get_param_count(mono_method_signature(method));
	assert(param_amount == parameter_count);
#endif // _DEBUG

	MonoObject* return_value = mono_runtime_invoke(method, mono_object, parameters, &exception);

	HandleException(exception);

	return return_value;
}

MonoClassHandle CSMonoCore::RegisterMonoClass(_MonoClass* mono_class)
{
	std::string class_name = mono_class_get_name(mono_class);
	std::string class_namespace = mono_class_get_namespace(mono_class);
	//Change this to an map instead or something, quite slow to go trough every class
	for (uint64_t i = 0; i < m_mono_classes.size(); ++i)
	{
		if (class_name == m_mono_classes[i].GetMonoClassName() && class_namespace == m_mono_classes[i].GetMonoNamespace())
		{
			return MonoClassHandle(i);
		}
	}

	return RegisterMonoClass(class_namespace, class_name);
}

void* CSMonoCore::GetValueInternal(const CSMonoObject& mono_object, const std::string& field_name)
{
	MonoClassField* field = mono_class_get_field_from_name(GetMonoClass(mono_object.m_class_handle)->GetMonoClass(), field_name.c_str());
	assert(field);

	return (void*)mono_field_get_value_object(m_domain, field, mono_object.GetMonoObject());
}

void CSMonoCore::SetValueInternal(const CSMonoObject& mono_object, const std::string& field_name, void* value)
{
	MonoClassField* field = mono_class_get_field_from_name(GetMonoClass(mono_object.m_class_handle)->GetMonoClass(), field_name.c_str());
	assert(field);

	mono_field_set_value(mono_object.GetMonoObject(), field, value);
}

void* CSMonoCore::GetValueInternal(const CSMonoObject& mono_object, const MonoFieldHandle& mono_field_handle)
{
	CSMonoClass* mono_class = GetMonoClass(mono_object.m_class_handle);
	MonoClassField* field = mono_class_get_field(mono_class->GetMonoClass(), mono_class->GetFieldToken(mono_field_handle));
	assert(field);

	return (void*)mono_field_get_value_object(m_domain, field, mono_object.GetMonoObject());
}

void CSMonoCore::SetValueInternal(const CSMonoObject& mono_object, const MonoFieldHandle& mono_field_handle, void* value)
{
	CSMonoClass* mono_class = GetMonoClass(mono_object.m_class_handle);
	MonoClassField* field = mono_class_get_field(mono_class->GetMonoClass(), mono_class->GetFieldToken(mono_field_handle));
	assert(field);

	mono_field_set_value(mono_object.GetMonoObject(), field, value);
}

MonoClassHandle CSMonoCore::RegisterMonoClass(const std::string& class_namespace, const std::string& class_name)
{
	std::string class_full_name = CSMonoClass::GetMonoClassFullName(class_namespace, class_name);
	if (m_mono_class_name_to_mono_class_handle.contains(class_full_name))
	{
		return m_mono_class_name_to_mono_class_handle.find(class_full_name)->second;
	}

	MonoClassHandle class_handle = { m_mono_classes.size() };

	m_mono_classes.push_back(CSMonoClass(this, class_namespace, class_name));

	m_mono_class_name_to_mono_class_handle.insert({ class_full_name, class_handle });

	return class_handle;
}

MonoMethodHandle CSMonoCore::RegisterMonoMethod(const MonoClassHandle& class_handle, const std::string& method_name)
{
	MonoMethodHandle method_handle = { m_mono_methods.size() };

	m_mono_methods.push_back(CSMonoMethod(GetMonoClass(class_handle), class_handle, method_name));

	return method_handle;
}

MonoFieldHandle CSMonoCore::RegisterField(const MonoClassHandle& mono_class_handle, const std::string& field_name)
{
	return GetMonoClass(mono_class_handle)->AddField(field_name);
}

bool CSMonoCore::CheckIfMonoMethodExists(const MonoClassHandle& class_handle, const std::string& method_name)
{
	return CSMonoMethod::CheckIfMonoMethodExists(GetMonoClass(class_handle), method_name);
}

bool CSMonoCore::CheckIfMonoMethodExists(const MonoMethodHandle& method_class)
{
	return method_class.handle < m_mono_methods.size();
}

MonoMethodHandle CSMonoCore::TryRegisterMonoMethod(const MonoClassHandle& class_handle, const std::string& method_name)
{
	MonoMethodHandle mono_method_handle = { (uint64_t)(-1) };
	if (CheckIfMonoMethodExists(class_handle, method_name))
	{
		return RegisterMonoMethod(class_handle, method_name);
	}
	return mono_method_handle;
}

MonoMethodHandle CSMonoCore::TryRegisterMonoMethod(const CSMonoObject& mono_object, const std::string& method_name)
{
	return TryRegisterMonoMethod(mono_object.m_class_handle, method_name);
}

void CSMonoCore::CallStaticMethod(const MonoMethodHandle& method_handle)
{
	CallMethodInternal(method_handle, nullptr, nullptr, 0);
}

void CSMonoCore::CallMethod(const MonoMethodHandle& method_handle, const CSMonoObject& mono_object)
{
	CallMethodInternal(method_handle, mono_object.GetMonoObject(), nullptr, 0);
}

void CSMonoCore::HookMethod(const MonoClassHandle& class_handle, const std::string& method_name, const void* method)
{
	CSMonoClass* mono_class = GetMonoClass(class_handle);
	std::string full_name = CSMonoMethod::GetMethodFullName(mono_class, method_name);
	mono_add_internal_call(full_name.c_str(), method);

	//MonoMethod* m;
	//void* iter = nullptr;
	//while ((m = mono_class_get_methods(mono_class->GetMonoClass(), &iter)))
	//{
	//	std::cout << mono_method_get_name(m) << "\n";
	//	std::cout << mono_method_full_name(m, true) << "\n";
	//}
}

MonoMethodHandle CSMonoCore::HookAndRegisterMonoMethod(const MonoClassHandle& class_handle, const std::string& method_name, const void* method)
{
	HookMethod(class_handle, method_name, method);
	return RegisterMonoMethod(class_handle, method_name);
}

CSMonoCore* CSMonoCore::Get()
{
	return s_mono_core;
}

void CSMonoCore::PrintMethod(const MonoMethodHandle& method_handle)
{
	std::cout << mono_method_full_name(GetMonoMethod(method_handle)->GetMonoMethod(), true) << "\n";
}
