#pragma once
#include "CSMonoHandles.h"

struct _MonoObject;
class CSMonoCore;

class CSMonoObject
{
	friend CSMonoCore;
private:
	uint32_t m_gchandle;
	MonoClassHandle m_class_handle;

	CSMonoCore* m_mono_core_ref;

	bool m_not_initialized = false;

private:
	_MonoObject* GetMonoObject() const;
	void CreateLinkToMono(_MonoObject* mono_object);

public:
	CSMonoObject(CSMonoCore* mono_core, const MonoClassHandle& class_handle);
	CSMonoObject(CSMonoCore* mono_core, _MonoObject* mono_object);
	CSMonoObject(const CSMonoObject& obj);
	CSMonoObject(CSMonoObject&& obj) noexcept;
	CSMonoObject();
	~CSMonoObject();

	CSMonoObject& operator=(const CSMonoObject& obj);
	CSMonoObject& operator=(CSMonoObject&& obj) noexcept;

	void RemoveLinkToMono();
};
