#pragma once

struct _MonoClass;
class CSMonoCore;

class CSMonoClass
{

private:
	std::string m_mono_namespace = "";
	std::string m_mono_class_name = "";
	_MonoClass* m_mono_class;

public:
	CSMonoClass(CSMonoCore* mono_core, const std::string& mono_namespace, const std::string& mono_class);

	const std::string& GetMonoClassName() const;
	_MonoClass* GetMonoClass() const;
};

