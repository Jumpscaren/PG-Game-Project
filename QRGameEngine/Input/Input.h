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
	static bool GetKeyPressed(int key);
	static bool GetKeyDown(int key);

	static bool GetMouseButtonPressed(int mouse_button);
	static bool GetMouseButtonDown(int mouse_button);

	static bool GetMouseWheelSpin(int direction);
};