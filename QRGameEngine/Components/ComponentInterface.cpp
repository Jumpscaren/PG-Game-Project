#include "pch.h"
#include "ComponentInterface.h"

void ComponentInterface::RegisterInterface(CSMonoCore* mono_core)
{
	mono_core->RegisterMonoClass("ScriptProject.Engine", "Component");
}

CSMonoObject ComponentInterface::GetGameObject(const CSMonoObject& component)
{
	CSMonoObject game_object;
	CSMonoCore::Get()->GetValue(game_object, component, "game_object");
	return game_object;
}
