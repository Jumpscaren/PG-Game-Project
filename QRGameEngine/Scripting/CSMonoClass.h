#pragma once
#include "CSMonoHandles.h"

struct _MonoClass;
class CSMonoCore;

class CSMonoClass
{

private:
	std::string m_mono_namespace = "";
	std::string m_mono_class_name = "";
	std::string m_mono_full_name = "";
	_MonoClass* m_mono_class;

	std::vector<uint32_t> m_field_handles;

public:
	CSMonoClass(CSMonoCore* mono_core, const std::string& mono_namespace, const std::string& mono_class);

	const std::string& GetMonoClassName() const;
	const std::string& GetMonoClassFullName() const;
	const std::string& GetMonoNamespace() const;
	_MonoClass* GetMonoClass() const;

	MonoFieldHandle AddField(const std::string& field_name);
	uint32_t GetFieldToken(const MonoFieldHandle& field_handle);
};

