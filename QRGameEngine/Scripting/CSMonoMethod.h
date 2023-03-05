#pragma once
#include "CSMonoHandles.h"

struct _MonoMethod;
class CSMonoClass;
class CSMonoCore;

class CSMonoMethod
{
	friend CSMonoCore;

private:
	MonoClassHandle m_class_handle;
	std::string m_method_name;
	_MonoMethod* m_mono_method;

private:
	static std::string GetMethodFullName(CSMonoClass* mono_class, const std::string& method_name);
	static _MonoMethod* GetMonoMethodFromMono(CSMonoClass* mono_class, const std::string& method_name);
	static bool CheckIfMonoMethodExists(CSMonoClass* mono_class, const std::string& method_name);

public:
	CSMonoMethod(CSMonoClass* mono_class, const MonoClassHandle& class_handle, const std::string& method_name);

	_MonoMethod* GetMonoMethod() const;
};

