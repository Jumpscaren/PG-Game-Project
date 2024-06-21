#pragma once
#include "Scripting/CSMonoHandles.h"

class CSMonoCore;

class TimeInterface
{
public:
	static void RegisterInterface(CSMonoCore* mono_core);

public:
	static void SetDeltaTime(CSMonoCore* mono_core);
	static void SetElapsedTime(CSMonoCore* mono_core);
	static float GetPreciseElapsedTime();

private:
	static MonoMethodHandle s_set_delta_time_method_handle;
	static MonoMethodHandle s_set_elapsed_time_method_handle;
};

