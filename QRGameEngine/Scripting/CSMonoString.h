#pragma once

struct _MonoString;

class CSMonoString
{
	_MonoString* m_mono_string;

public:
	CSMonoString(_MonoString* mono_string);

	CSMonoString(const CSMonoString& s);

	operator std::string() const;
};

