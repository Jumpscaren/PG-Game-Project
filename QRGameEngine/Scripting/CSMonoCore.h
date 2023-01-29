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

public:
	CSMonoCore();

public:
	MonoClassHandle RegisterMonoClass(const std::string& class_namespace, const std::string& class_name);
	MonoMethodHandle RegisterMonoMethod(const MonoClassHandle& class_handle, const std::string& method_name);

	void CallMethod(const MonoMethodHandle& method_handle);
	void CallMethod(CSMonoObject* mono_object, const MonoMethodHandle& method_handle);

	void HookMethod(const MonoClassHandle& class_handle, const std::string& method_name, const void* method);
	MonoMethodHandle HookAndRegisterMonoMethod(const MonoClassHandle& class_handle, const std::string& method_name, const void* method);
};
