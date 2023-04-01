#pragma once
#include "Scripting/CSMonoCore.h"

class ComponentInterface
{
public:
	static void RegisterInterface(CSMonoCore* mono_core);
	
public:
	static CSMonoObject GetGameObject(const CSMonoObject& component);
};

