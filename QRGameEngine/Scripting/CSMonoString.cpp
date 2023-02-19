#include "pch.h"
#include "CSMonoString.h"
#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/debug-helpers.h>

CSMonoString::CSMonoString(_MonoString* mono_string) : m_mono_string(mono_string)
{

}

CSMonoString::CSMonoString(const CSMonoString& s) : m_mono_string(s.m_mono_string)
{
}

CSMonoString::operator std::string() const
{
	std::string string = mono_string_to_utf8(m_mono_string);
	return std::move(string);
}
