#include "pch.h"
#include "MonoCore.h"

MonoCore::MonoCore()
{
	mono_set_dirs("../QRGameEngine/", "../QRGameEngine/");

	m_domain = mono_jit_init("MonoCore");

	assert(m_domain);

	m_assembly = mono_domain_assembly_open(m_domain, "../ScriptProject/build/ScriptProject.dll");

	assert(m_assembly);

	m_image = mono_assembly_get_image(m_assembly);

	assert(m_image);
}
