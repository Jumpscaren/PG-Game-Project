#pragma once
#include "CSMonoClass.h"
#include "CSMonoMethod.h"
#include "CSMonoObject.h"

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
	void* ToMethodParameter(CSMonoObject* mono_object);

	int MonoObjectToValue(int* mono_object);
	float MonoObjectToValue(float* mono_object);
	double MonoObjectToValue(double* mono_object);
	bool MonoObjectToValue(bool* mono_object);
	std::string MonoObjectToValue(std::string* mono_object);
	CSMonoObject* MonoObjectToValue(CSMonoObject** mono_object);

	_MonoObject* CallMethodInternal(const MonoMethodHandle& method_handle, CSMonoObject* mono_object, void** parameters, uint32_t parameter_count);

	MonoClassHandle RegisterMonoClass(_MonoClass* mono_class);

public:
	CSMonoCore();

public:
	MonoClassHandle RegisterMonoClass(const std::string& class_namespace, const std::string& class_name);
	MonoMethodHandle RegisterMonoMethod(const MonoClassHandle& class_handle, const std::string& method_name);

	void CallMethod(const MonoMethodHandle& method_handle);
	void CallMethod(const MonoMethodHandle& method_handle, CSMonoObject* mono_object);
	template<typename...Args>
	void CallMethod(const MonoMethodHandle& method_handle, CSMonoObject* mono_object, Args&& ...args);

	template<typename Type>
	void CallMethod(Type& return_value, const MonoMethodHandle& method_handle, CSMonoObject* mono_object);
	template<typename Type, typename...Args>
	void CallMethod(Type& return_value, const MonoMethodHandle& method_handle, CSMonoObject* mono_object, Args&& ...args);

	void HookMethod(const MonoClassHandle& class_handle, const std::string& method_name, const void* method);
	MonoMethodHandle HookAndRegisterMonoMethod(const MonoClassHandle& class_handle, const std::string& method_name, const void* method);
};

template<typename ...Args>
inline void CSMonoCore::CallMethod(const MonoMethodHandle& method_handle, CSMonoObject* mono_object, Args && ...args)
{
	const int argument_size = sizeof...(Args);

	void* parameters[argument_size];
	
	int index = 0;

	((parameters[index++] = ToMethodParameter(args)), ...);

	CallMethodInternal(method_handle, mono_object, parameters, index);
}

template<typename Type>
inline void CSMonoCore::CallMethod(Type& return_value, const MonoMethodHandle& method_handle, CSMonoObject* mono_object)
{
	void** parameters = nullptr;

	_MonoObject* method_return_value = CallMethodInternal(method_handle, mono_object, parameters, 0);

	return_value = std::move(MonoObjectToValue((Type*)method_return_value));
}

template<typename Type, typename ...Args>
inline void CSMonoCore::CallMethod(Type& return_value, const MonoMethodHandle& method_handle, CSMonoObject* mono_object, Args && ...args)
{
	const int argument_size = sizeof...(Args);

	void* parameters[argument_size];

	int index = 0;

	((parameters[index++] = ToMethodParameter(args)), ...);

	_MonoObject* method_return_value = CallMethodInternal(method_handle, mono_object, parameters, index);

	return_value = std::move(MonoObjectToValue((Type*)method_return_value));
}
