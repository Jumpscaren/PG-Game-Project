#pragma once
#include "CSMonoHandles.h"

struct _MonoMethod;
class CSMonoClass;

class CSMonoMethod
{
private:
	MonoClassHandle m_class_handle;
	std::string m_method_name;
	_MonoMethod* m_mono_method;

public:
	CSMonoMethod(CSMonoClass* mono_class, const MonoClassHandle& class_handle, const std::string& method_name);

	_MonoMethod* GetMonoMethod() const;
};

