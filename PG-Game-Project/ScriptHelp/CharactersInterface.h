#pragma once
#include "Scripting/CSMonoObject.h"

class CharactersInterface
{
public:
	static void RegisterInterface();

private:
	static void GetInteractiveCharacters(const CSMonoObject& list);
};

