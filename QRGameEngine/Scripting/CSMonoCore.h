#pragma once
#include "CSMonoClass.h"
#include "CSMonoMethod.h"
#include "CSMonoObject.h"
#include "CSMonoString.h"

struct _MonoDomain;
struct _MonoAssembly;
struct _MonoImage;

class CSMonoCore
{
	friend CSMonoObject;
	friend CSMonoClass;

private:
	_MonoDomain* m_domain;
	_MonoAssembly* m_assembly;
	_MonoImage* m_image;

	std::vector<CSMonoClass> m_mono_classes;
	std::vector<CSMonoMethod> m_mono_methods;

	static CSMonoCore* s_mono_core;

private:
	//Type changing templates
	//Check out tuple from STD to check where this code came from

	/* 
	* Reason behind this is to let the user have simple types like string and not MonoString or CSMonoString.
	* With no type changing then the user would need to have pointers to a CSMonoString and CSMonoObject which could lead to memory leaks, in their functions which they hook.
	* The user would be responsible for removing the objects!
	* This is also much cleaner ;).
	*/

	//Base struct
	template <char h, class T = void>
	struct TypeChange {};

	//If the type is a string then we want a _MonoString*
	template<>
	struct TypeChange<0, std::string>
	{
		using type = _MonoString*;
	};

	//If the type is a CSMonoObject then we want a _MonoObject*
	template<>
	struct TypeChange<0, CSMonoObject>
	{
		using type = _MonoObject*;
	};

	//If the type is a _MonoString* then we want a string
	template<>
	struct TypeChange<0, _MonoString*>
	{
		using type = std::string;
	};

	//If the type is a _MonoObject* then we want a CSMonoObject
	template<>
	struct TypeChange<0, _MonoObject*>
	{
		using type = CSMonoObject;
	};

	//This is for types which we do not need to change
	template <class T>
	struct TypeChange<0, T>
	{
		using type = T;
	};

	template<class T>
	using ChangeType = typename TypeChange<0, T>::type;

private:
	CSMonoClass* GetMonoClass(const MonoClassHandle& class_handle);
	CSMonoMethod* GetMonoMethod(const MonoMethodHandle& method_handle);
	_MonoDomain* GetDomain() const;
	_MonoImage* GetImage() const;

	void HandleException(_MonoObject* exception);

	void* ToMethodParameter(int& number);
	void* ToMethodParameter(float& number);
	void* ToMethodParameter(double& number);
	void* ToMethodParameter(bool& boolean);
	void* ToMethodParameter(const char* string);
	void* ToMethodParameter(const std::string& string);
	void* ToMethodParameter(const CSMonoObject& mono_object);

	int MonoObjectToValue(int* mono_object);
	float MonoObjectToValue(float* mono_object);
	double MonoObjectToValue(double* mono_object);
	bool MonoObjectToValue(bool* mono_object);
	std::string MonoObjectToValue(std::string* mono_object);
	CSMonoObject MonoObjectToValue(CSMonoObject* mono_object);

	static int MonoMethodParameter(int mono_parameter);
	static float MonoMethodParameter(float mono_parameter);
	static double MonoMethodParameter(double mono_parameter);
	static bool MonoMethodParameter(bool mono_parameter);
	static std::string MonoMethodParameter(_MonoString* mono_parameter);
	static CSMonoObject MonoMethodParameter(_MonoObject* mono_parameter);

	_MonoObject* CallMethodInternal(const MonoMethodHandle& method_handle, _MonoObject* mono_object, void** parameters, uint32_t parameter_count);

	MonoClassHandle RegisterMonoClass(_MonoClass* mono_class);

	void* GetValueInternal(const CSMonoObject& mono_object, const std::string& field_name);
	void SetValueInternal(const CSMonoObject& mono_object, const std::string& field_name, void* value);
	void* GetValueInternal(const CSMonoObject& mono_object, const MonoFieldHandle& mono_field_handle);
	void SetValueInternal(const CSMonoObject& mono_object, const MonoFieldHandle& mono_field_handle, void* value);

public:
	CSMonoCore();

public:
	MonoClassHandle RegisterMonoClass(const std::string& class_namespace, const std::string& class_name);
	MonoMethodHandle RegisterMonoMethod(const MonoClassHandle& class_handle, const std::string& method_name);
	MonoFieldHandle RegisterField(const MonoClassHandle& mono_class_handle, const std::string& field_name);

	template<typename Type>
	void GetValue(Type& return_value, const CSMonoObject& mono_object, const std::string& field_name);
	template<typename Type>
	void SetValue(Type& value_to_set, const CSMonoObject& mono_object, const std::string& field_name);
	template<typename Type>
	void GetValue(Type& return_value, const CSMonoObject& mono_object, const MonoFieldHandle& mono_field_handle);
	template<typename Type>
	void SetValue(Type& value_to_set, const CSMonoObject& mono_object, const MonoFieldHandle& mono_field_handle);

	void CallStaticMethod(const MonoMethodHandle& method_handle);
	void CallMethod(const MonoMethodHandle& method_handle, const CSMonoObject& mono_object);

	template<typename...Args>
	void CallStaticMethod(const MonoMethodHandle& method_handle, Args&& ...args);
	template<typename...Args>
	void CallMethod(const MonoMethodHandle& method_handle, const CSMonoObject& mono_object, Args&& ...args);

	template<typename Type>
	void CallStaticMethod(Type& return_value, const MonoMethodHandle& method_handle);
	template<typename Type>
	void CallMethod(Type& return_value, const MonoMethodHandle& method_handle, const CSMonoObject& mono_object);

	template<typename Type, typename...Args>
	void CallStaticMethod(Type& return_value, const MonoMethodHandle& method_handle, Args&& ...args);
	template<typename Type, typename...Args>
	void CallMethod(Type& return_value, const MonoMethodHandle& method_handle, const CSMonoObject& mono_object, Args&& ...args);

	void HookMethod(const MonoClassHandle& class_handle, const std::string& method_name, const void* method);
	MonoMethodHandle HookAndRegisterMonoMethod(const MonoClassHandle& class_handle, const std::string& method_name, const void* method);

	template<auto t_method, typename Type, typename...Args>
	MonoMethodHandle HookAndRegisterMonoMethodType(const MonoClassHandle& class_handle, const std::string& method_name, Type(*)(Args...));

	template<void* method, typename Type, typename...Args>
	static Type HookedMethod(Args... args);
};

template<typename Type>
inline void CSMonoCore::GetValue(Type& return_value, const CSMonoObject& mono_object, const std::string& field_name)
{
	void* value = GetValueInternal(mono_object, field_name);
	return_value = MonoObjectToValue((Type*)value);
}

template<typename Type>
inline void CSMonoCore::SetValue(Type& value_to_set, const CSMonoObject& mono_object, const std::string& field_name)
{
	void* value = ToMethodParameter(value_to_set);
	SetValueInternal(mono_object, field_name, value);
}

template<typename Type>
inline void CSMonoCore::GetValue(Type& return_value, const CSMonoObject& mono_object, const MonoFieldHandle& mono_field_handle)
{
	void* value = GetValueInternal(mono_object, mono_field_handle);
	return_value = MonoObjectToValue((Type*)value);
}

template<typename Type>
inline void CSMonoCore::SetValue(Type& value_to_set, const CSMonoObject& mono_object, const MonoFieldHandle& mono_field_handle)
{
	void* value = ToMethodParameter(value_to_set);
	SetValueInternal(mono_object, mono_field_handle, value);
}

template<typename ...Args>
inline void CSMonoCore::CallStaticMethod(const MonoMethodHandle& method_handle, Args && ...args)
{
	const int argument_size = sizeof...(Args);

	void* parameters[argument_size];

	int index = 0;

	((parameters[index++] = ToMethodParameter(args)), ...);

	CallMethodInternal(method_handle, nullptr, parameters, index);
}

template<typename ...Args>
inline void CSMonoCore::CallMethod(const MonoMethodHandle& method_handle, const CSMonoObject& mono_object, Args && ...args)
{
	const int argument_size = sizeof...(Args);

	void* parameters[argument_size];
	
	int index = 0;

	((parameters[index++] = ToMethodParameter(args)), ...);

	CallMethodInternal(method_handle, mono_object.GetMonoObject(), parameters, index);
}

template<typename Type>
inline void CSMonoCore::CallStaticMethod(Type& return_value, const MonoMethodHandle& method_handle)
{
	void** parameters = nullptr;

	_MonoObject* method_return_value = CallMethodInternal(method_handle, nullptr, parameters, 0);

	return_value = std::move(MonoObjectToValue((Type*)method_return_value));
}

template<typename Type>
inline void CSMonoCore::CallMethod(Type& return_value, const MonoMethodHandle& method_handle, const CSMonoObject& mono_object)
{
	void** parameters = nullptr;

	_MonoObject* method_return_value = CallMethodInternal(method_handle, mono_object.GetMonoObject(), parameters, 0);

	return_value = std::move(MonoObjectToValue((Type*)method_return_value));
}

template<typename Type, typename ...Args>
inline void CSMonoCore::CallStaticMethod(Type& return_value, const MonoMethodHandle& method_handle, Args && ...args)
{
	const int argument_size = sizeof...(Args);

	void* parameters[argument_size];

	int index = 0;

	((parameters[index++] = ToMethodParameter(args)), ...);

	_MonoObject* method_return_value = CallMethodInternal(method_handle, nullptr, parameters, index);

	return_value = std::move(MonoObjectToValue((Type*)method_return_value));
}

template<typename Type, typename ...Args>
inline void CSMonoCore::CallMethod(Type& return_value, const MonoMethodHandle& method_handle, const CSMonoObject& mono_object, Args && ...args)
{
	const int argument_size = sizeof...(Args);

	void* parameters[argument_size];

	int index = 0;

	((parameters[index++] = ToMethodParameter(args)), ...);

	_MonoObject* method_return_value = CallMethodInternal(method_handle, mono_object.GetMonoObject(), parameters, index);

	return_value = std::move(MonoObjectToValue((Type*)method_return_value));
}

template<auto t_method, typename Type, typename ...Args>
inline MonoMethodHandle CSMonoCore::HookAndRegisterMonoMethodType(const MonoClassHandle& class_handle, const std::string& method_name, Type(*)(Args...))
{
	return HookAndRegisterMonoMethod(class_handle, method_name, &CSMonoCore::HookedMethod<(void*)t_method, Type, ChangeType<Args>...>);
}

template<void* method, typename Type, typename ...Args>
inline Type CSMonoCore::HookedMethod(Args ...args)
{
	return ((Type(*)(ChangeType<Args>...))(method))(MonoMethodParameter(args)...);
}
