#include "pch.h"
#include "TimeInterface.h"
#include "Scripting/CSMonoCore.h"
#include "Time/Time.h"

MonoMethodHandle TimeInterface::s_set_delta_time_method_handle;
MonoMethodHandle TimeInterface::s_set_elapsed_time_method_handle;

void TimeInterface::RegisterInterface(CSMonoCore* mono_core)
{
	auto time_class = mono_core->RegisterMonoClass("ScriptProject.Engine", "Time");

	s_set_delta_time_method_handle = mono_core->RegisterMonoMethod(time_class, "SetDeltaTime");
	s_set_elapsed_time_method_handle = mono_core->RegisterMonoMethod(time_class, "SetElapsedTime");
	mono_core->HookAndRegisterMonoMethodType<TimeInterface::GetPreciseElapsedTime>(time_class, "GetPreciseElapsedTime", TimeInterface::GetPreciseElapsedTime);
}

void TimeInterface::SetDeltaTime(CSMonoCore* mono_core)
{
	mono_core->CallStaticMethod(s_set_delta_time_method_handle, (float)Time::GetDeltaTime());
}

void TimeInterface::SetElapsedTime(CSMonoCore* mono_core)
{
	mono_core->CallStaticMethod(s_set_elapsed_time_method_handle, (float)Time::GetElapsedTime());
}

float TimeInterface::GetPreciseElapsedTime()
{
	return (float)Time::GetElapsedTime();
}
