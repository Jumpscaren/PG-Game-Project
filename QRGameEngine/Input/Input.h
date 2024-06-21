#pragma once
#include "Common/EngineTypes.h"
#include "Scripting/CSMonoObject.h"

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

	static CSMonoObject GetMousePosition();
	static CSMonoObject GetMousePositionInWorld(const CSMonoObject& camera_game_object);
};