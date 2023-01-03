#pragma once
#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/debug-helpers.h>

class MonoCore
{
private:
	MonoDomain* m_domain;
	MonoAssembly* m_assembly;
	MonoImage* m_image;

public:
	MonoCore();
};

