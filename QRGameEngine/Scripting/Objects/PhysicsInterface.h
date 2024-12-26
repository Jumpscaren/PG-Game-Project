#pragma once
#include "Scripting/CSMonoObject.h"
#include "SceneSystem/SceneDefines.h"
#include "ECS/EntityDefinition.h"

class PhysicsInterface
{
private:
	static MonoClassHandle s_raycast_result_struct;
	static MonoFieldHandle s_intersected_field;
	static MonoFieldHandle s_intersected_game_object_field;
	static MonoFieldHandle s_intersected_position_field;

public:
	static void RegisterInterface(CSMonoCore* mono_core);

public:
	static CSMonoObject Raycast(const CSMonoObject& position, const CSMonoObject& direction, uint16_t category, uint16_t mask, int16_t group);
	static bool RaycastCheckIfClosest(const CSMonoObject& position, const CSMonoObject& direction, uint16_t category, uint16_t mask, int16_t group, SceneIndex scene_index, Entity entity);
};

