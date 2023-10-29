#pragma once

class CSMonoCore;

class TimeInterface
{
public:
	static void RegisterInterface(CSMonoCore* mono_core);

public:
	static float GetDeltaTime();
	static float GetElapsedTime();
};

