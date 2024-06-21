#pragma once
#include "CSMonoHandles.h"

struct _MonoObject;
struct _MonoClass;
class CSMonoCore;

class CSMonoObject
{
	friend CSMonoCore;
private:
	uint32_t m_gchandle = -1;
	//MonoClassHandle m_class_handle;
	_MonoClass* m_mono_class;

	CSMonoCore* m_mono_core_ref;

	bool m_not_initialized = false;

	static std::unordered_set<uint32_t> test;

private:
	_MonoObject* GetMonoObject() const;
	void CreateLinkToMono(_MonoObject* mono_object);

public:
	CSMonoObject(CSMonoCore* mono_core, const MonoClassHandle class_handle);
	CSMonoObject(CSMonoCore* mono_core, _MonoObject* mono_object);
	CSMonoObject(const CSMonoObject& obj);
	CSMonoObject(CSMonoObject&& obj) noexcept;
	CSMonoObject();
	~CSMonoObject();

	CSMonoObject& operator=(const CSMonoObject& obj);
	CSMonoObject& operator=(CSMonoObject&& obj) noexcept;

	void RemoveLinkToMono();

	static std::string GetName(const uint32_t gchandle);
};
