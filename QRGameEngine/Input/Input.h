#pragma once

class CSMonoCore;

class Input
{
};

class InputInterface
{
public:
	static void RegisterInterface(CSMonoCore* mono_core);

private:
	static bool GetKeyPressed();
	static bool GetKeyDown();
};