#include "pch.h"
#include "TimeInterface.h"
#include "Scripting/CSMonoCore.h"
#include "Time/Time.h"

void TimeInterface::RegisterInterface(CSMonoCore* mono_core)
{
	auto time_class = mono_core->RegisterMonoClass("ScriptProject.Engine", "Time");

	mono_core->HookAndRegisterMonoMethodType<TimeInterface::GetDeltaTime>(time_class, "GetDeltaTime", TimeInterface::GetDeltaTime);
	mono_core->HookAndRegisterMonoMethodType<TimeInterface::GetElapsedTime>(time_class, "GetElapsedTime", TimeInterface::GetElapsedTime);
}

float TimeInterface::GetDeltaTime()
{
	return (float)Time::GetDeltaTime();
}

float TimeInterface::GetElapsedTime()
{
	return (float)Time::GetElapsedTime();
}
