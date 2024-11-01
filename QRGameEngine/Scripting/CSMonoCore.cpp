#include "pch.h"
#include "CSMonoCore.h"
#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/debug-helpers.h>
#include <mono/metadata/mono-config.h>
#include <mono/metadata/mono-gc.h>
#include <mono/metadata/threads.h>

CSMonoCore* CSMonoCore::s_mono_core = nullptr;

CSMonoClass* CSMonoCore::GetMonoClass(const MonoClassHandle class_handle)
{
	return &m_mono_classes[class_handle.handle];
}

CSMonoMethod* CSMonoCore::GetMonoMethod(const MonoMethodHandle method_handle)
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

	//MonoClass* listGenericClass = mono_class_from_name(m_image, "System.Collections.Generic", "List");
	//if (!listGenericClass) {
	//	std::cerr << "Failed to find List<T> class" << std::endl;
	//	//return 1;
	//}

	//// Get the MonoClass for System.Int32
	//MonoClass* intClass = mono_class_from_name(m_image, "System", "Int32");
	//if (!intClass) {
	//	std::cerr << "Failed to find System.Int32 class" << std::endl;
	//	//return 1;
	//}

	//MonoReflectionType* intType = mono_type_get_object(m_domain, mono_class_get_type(intClass));
	//MonoArray* typeArgs = mono_array_new(m_domain, mono_get_object_class(), 1);
	//mono_array_set(typeArgs, MonoReflectionType*, 0, intType);

	//MonoClass* genericListClass = mono_class_from_name(m_image, "System.Collections.Generic", "List`1");
	//MonoReflectionType* listGenericType = mono_type_get_object(m_domain, mono_class_get_type(genericListClass));
	//MonoObject* listType = mono_runtime_invoke(mono_class_get_method_from_name(listGenericClass, "MakeGenericType", 1), listGenericType, (void**)&typeArgs, NULL);

	//// Create an instance of List<int>
	//MonoObject* listInstance = mono_object_new(m_domain, mono_type_get_class(mono_reflection_type_get_type((MonoReflectionType*)listType)));
	//mono_runtime_object_init(listInstance);

	//// Get the method for List<int>.Add(int)
	//MonoMethodDesc* addMethodDesc = mono_method_desc_new("System.Collections.Generic.List`1:Add(System.Int32)", false);
	//MonoMethod* addMethod = mono_method_desc_search_in_class(addMethodDesc, mono_object_get_class(listInstance));
	//if (!addMethod) {
	//	std::cerr << "Failed to get Add method" << std::endl;
	//	//return 1;
	//}

	//// Add elements to the list
	//for (int i = 0; i < 10; ++i) {
	//	void* args[1];
	//	args[0] = &i;
	//	mono_runtime_invoke(addMethod, listInstance, args, NULL);
	//}
}

CSMonoCore::~CSMonoCore()
{
	//mono_images_cleanup();
	//mono_assemblies_cleanup();
	//mono_domain_free(m_domain, true);
	//mono_assembly_close(m_assembly);
	//mono_domain_free(m_domain, true);
	//mono_domain_free();
	mono_jit_cleanup(m_domain);
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

void* CSMonoCore::UnboxMonoObject(_MonoObject* mono_object)
{
	return mono_object_unbox((MonoObject*)mono_object);
}

std::string CSMonoCore::MonoObjectToValue(std::string* mono_object)
{
	MonoString* const mono_string = mono_object_to_string((MonoObject*)mono_object, nullptr);
	std::string return_string = MonoStringToString(mono_string);
	//mono_string_length(mono_string); // This is necessary to notify the runtime
	//mono_free(mono_string);
	return return_string;
}

CSMonoObject CSMonoCore::MonoObjectToValue(CSMonoObject* mono_object)
{
	return CSMonoObject(this, (MonoObject*)(mono_object));
}

std::string CSMonoCore::MonoMethodParameter(_MonoString* mono_parameter)
{
	return MonoStringToString(mono_parameter);
}

std::string CSMonoCore::MonoMethodParameter(ConstMonoString* mono_parameter)
{
	return MonoStringToString((_MonoString*)mono_parameter);
}

CSMonoObject CSMonoCore::MonoMethodParameter(_MonoObject* mono_parameter)
{
	return CSMonoObject(s_mono_core, (MonoObject*)(mono_parameter));
}

CSMonoObject CSMonoCore::MonoMethodParameter(ConstMonoObject* mono_parameter)
{
	return CSMonoObject(s_mono_core, (MonoObject*)(mono_parameter));
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
	assert(m_mono_class_ptr_to_mono_class_handle.size() <= m_mono_classes.size());

	const auto it = m_mono_class_ptr_to_mono_class_handle.find(mono_class);
	if (it != m_mono_class_ptr_to_mono_class_handle.end())
	{
		return it->second;
	}

	std::string class_name = mono_class_get_name(mono_class);
	std::string class_namespace = mono_class_get_namespace(mono_class);

	//Change this to an map instead or something, quite slow to go trough every class
	for (uint64_t i = 0; i < m_mono_classes.size(); ++i)
	{
		if (class_name == m_mono_classes[i].GetMonoClassName() && class_namespace == m_mono_classes[i].GetMonoNamespace())
		{
			m_mono_class_ptr_to_mono_class_handle.emplace(mono_class, MonoClassHandle(i));
			return MonoClassHandle(i);
		}
	}

	const auto mono_class_handle = RegisterMonoClass(class_namespace, class_name);
	m_mono_class_ptr_to_mono_class_handle.emplace(mono_class, mono_class_handle);
	return mono_class_handle;
}

bool CSMonoCore::IsValueTypeInternal(const CSMonoType cs_mono_type, const CSMonoObject& mono_object, const std::string& field_name)
{
	MonoClassField* mono_field = mono_class_get_field_from_name(mono_object.m_mono_class, field_name.c_str());
	MonoType* mono_type = mono_field_get_type(mono_field);
	const int mono_type_enum = mono_type_get_type(mono_type);
	return mono_type_enum == (int)cs_mono_type;
}

void* CSMonoCore::GetValueInternal(const CSMonoObject& mono_object, const std::string& field_name)
{
	MonoClassField* field = mono_class_get_field_from_name(mono_object.m_mono_class, field_name.c_str());
	assert(field);

	return (void*)mono_field_get_value_object(m_domain, field, mono_object.GetMonoObject());
}

void CSMonoCore::SetValueInternal(const CSMonoObject& mono_object, const std::string& field_name, void* value)
{
	MonoClassField* field = mono_class_get_field_from_name(mono_object.m_mono_class, field_name.c_str());
	assert(field);

	mono_field_set_value(mono_object.GetMonoObject(), field, value);
}

void* CSMonoCore::GetValueInternal(const CSMonoObject& mono_object, const MonoFieldHandle& mono_field_handle)
{
	//CSMonoClass* mono_class = GetMonoClass(mono_object.m_class_handle);
	//MonoClassField* field = mono_class_get_field(mono_class->GetMonoClass(), mono_class->GetFieldToken(mono_field_handle));
	assert(mono_field_handle.handle < m_mono_field_tokens.size());
	MonoClassField* field = mono_class_get_field(mono_object.m_mono_class, m_mono_field_tokens[mono_field_handle.handle]);
	assert(field);

	return (void*)mono_field_get_value_object(m_domain, field, mono_object.GetMonoObject());
}

void CSMonoCore::SetValueInternal(const CSMonoObject& mono_object, const MonoFieldHandle& mono_field_handle, void* value)
{
	//CSMonoClass* mono_class = GetMonoClass(mono_object.m_class_handle);
	assert(mono_field_handle.handle < m_mono_field_tokens.size());
	MonoClassField* field = mono_class_get_field(mono_object.m_mono_class, m_mono_field_tokens[mono_field_handle.handle]);
	assert(field);

	mono_field_set_value(mono_object.GetMonoObject(), field, value);
}

std::string CSMonoCore::MonoStringToString(_MonoString* mono_string)
{
	//We need to remove this data ourselves //this caused an entire day of debugging :)
	char* char_string = mono_string_to_utf8(mono_string);
	std::string string = char_string;
	mono_string_length(mono_string); // This is necessary to notify the runtime
	mono_free(char_string);
	return string;
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
	std::string method_full_name = CSMonoMethod::GetMethodFullName(GetMonoClass(class_handle), method_name);
	if (m_mono_method_name_to_mono_method_handle.contains(method_full_name))
	{
		return m_mono_method_name_to_mono_method_handle.find(method_full_name)->second;
	}

	MonoMethodHandle method_handle = { m_mono_methods.size() };

	//auto mono_class = GetMonoClass(class_handle);
	//MonoMethod* m;
	//void* iter = nullptr;
	//while ((m = mono_class_get_methods(mono_class->GetMonoClass(), &iter)))
	//{
	//	std::cout << mono_method_get_name(m) << "\n";
	//	std::cout << mono_method_full_name(m, true) << "\n";
	//}

	m_mono_methods.push_back(CSMonoMethod(GetMonoClass(class_handle), class_handle, method_name));
	m_mono_method_name_to_mono_method_handle.insert({method_full_name, method_handle});
	return method_handle;
}

MonoFieldHandle CSMonoCore::RegisterField(const MonoClassHandle& mono_class_handle, const std::string& field_name)
{
	MonoFieldHandle field_handle = {};
	field_handle.handle = m_mono_field_tokens.size();

	MonoClassField* field = mono_class_get_field_from_name(GetMonoClass(mono_class_handle)->GetMonoClass(), field_name.c_str());
	assert(field);

	uint32_t token = mono_class_get_field_token(field);
	m_mono_field_tokens.push_back(token);

	return field_handle;
	//return GetMonoClass(mono_class_handle)->AddField(field_name);
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
	MonoMethodHandle mono_method_handle = NULL_METHOD;
	if (CheckIfMonoMethodExists(class_handle, method_name))
	{
		return RegisterMonoMethod(class_handle, method_name);
	}
	return mono_method_handle;
}

MonoMethodHandle CSMonoCore::TryRegisterMonoMethod(const CSMonoObject& mono_object, const std::string& method_name)
{
	MonoMethodHandle mono_method_handle = NULL_METHOD;
	const auto mono_class_handle = RegisterMonoClass(mono_object.m_mono_class);
	if (CheckIfMonoMethodExists(mono_class_handle, method_name))
	{
		return RegisterMonoMethod(mono_class_handle, method_name);
	}
	return mono_method_handle;
}

MonoClassHandle CSMonoCore::TryGetParentClass(const CSMonoObject& mono_object)
{
	if (auto* parent = mono_class_get_parent(mono_object.m_mono_class); parent)
	{
		 return RegisterMonoClass(parent);
	}
	return NULL_CLASS;
}

void CSMonoCore::CallStaticMethod(const MonoMethodHandle method_handle)
{
	CallMethodInternal(method_handle, nullptr, nullptr, 0);
}

void CSMonoCore::CallMethod(const MonoMethodHandle method_handle, const CSMonoObject& mono_object)
{
	CallMethodInternal(method_handle, mono_object.GetMonoObject(), nullptr, 0);
}

void CSMonoCore::HookMethod(const MonoClassHandle class_handle, const std::string& method_name, const void* method)
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

std::vector<std::string> CSMonoCore::GetAllFieldNames(const CSMonoObject& mono_object)
{
	std::vector<std::string> field_names;
	field_names.resize(mono_class_num_fields(mono_object.m_mono_class));

	MonoClassField* field;
	void* iter = nullptr;
	uint32_t i = 0;
	while ((field = mono_class_get_fields(mono_object.m_mono_class, &iter)))
	{
		field_names[i++] = mono_field_get_name(field);
	}

	return field_names;
}

void CSMonoCore::PrintAllMethodsFromClass(const MonoClassHandle& class_handle)
{
	CSMonoClass* mono_class = GetMonoClass(class_handle);
	MonoMethod* m;
	void* iter = nullptr;
	while ((m = mono_class_get_methods(mono_class->GetMonoClass(), &iter)))
	{
		std::cout << mono_method_full_name(m, true) << "\n";
	}
}

void CSMonoCore::PrintMethod(const MonoMethodHandle& method_handle)
{
	std::cout << mono_method_full_name(GetMonoMethod(method_handle)->GetMonoMethod(), true) << "\n";
}

void CSMonoCore::ForceGarbageCollection()
{
	mono_gc_collect(mono_gc_max_generation());
}

void CSMonoCore::HookThread()
{
	m_thread = mono_thread_attach(m_domain);
}

void CSMonoCore::UnhookThread()
{
	mono_thread_detach(m_thread);
}
