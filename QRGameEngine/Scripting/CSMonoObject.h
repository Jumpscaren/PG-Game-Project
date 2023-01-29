#pragma once
#include "CSMonoHandles.h"

struct _MonoObject;
class CSMonoCore;

class CSMonoObject
{
	friend CSMonoCore;
private:
	_MonoObject* m_mono_object;
	uint32_t m_gchandle;
	MonoClassHandle m_class_handle;

private:
	_MonoObject* GetMonoObject() const;

public:
	CSMonoObject(CSMonoCore* mono_core, const MonoClassHandle& class_handle);
	~CSMonoObject();
};

