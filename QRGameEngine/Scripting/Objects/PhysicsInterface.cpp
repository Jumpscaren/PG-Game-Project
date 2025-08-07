#include "pch.h"
#include "PhysicsInterface.h"
#include "Scripting/CSMonoCore.h"
#include "Physics/PhysicsCore.h"
#include "Scripting/Objects/Vector2Interface.h"
#include "GameObjectInterface.h"

MonoClassHandle PhysicsInterface::s_raycast_result_struct;
MonoFieldHandle PhysicsInterface::s_intersected_field;
MonoFieldHandle PhysicsInterface::s_intersected_game_object_field;
MonoFieldHandle PhysicsInterface::s_intersected_position_field;

void PhysicsInterface::RegisterInterface(CSMonoCore* mono_core)
{
	const auto physics_class = mono_core->RegisterMonoClass("ScriptProject.Engine", "Physics");

	mono_core->HookAndRegisterMonoMethodType<PhysicsInterface::Raycast>(physics_class, "Raycast", PhysicsInterface::Raycast);
	mono_core->HookAndRegisterMonoMethodType<PhysicsInterface::RaycastCheckIfClosest>(physics_class, "RaycastCheckIfClosest_Extern", PhysicsInterface::RaycastCheckIfClosest);

	s_raycast_result_struct = mono_core->RegisterMonoClass("ScriptProject.Engine", "RaycastResult");
	s_intersected_field = mono_core->RegisterField(s_raycast_result_struct, "intersected");
	s_intersected_game_object_field = mono_core->RegisterField(s_raycast_result_struct, "intersected_game_object");
	s_intersected_position_field = mono_core->RegisterField(s_raycast_result_struct, "intersected_position");
}

CSMonoObject PhysicsInterface::Raycast(const CSMonoObject& position, const CSMonoObject& direction,
	const uint16_t category, const uint16_t mask, const int16_t group)
{
	const ColliderFilter collider_filter{.category_bits = category, .mask_bits = mask, .group_index = group};
	const RaycastResult result =
		PhysicsCore::Get()->Raycast(Vector2Interface::GetVector2(position), Vector2Interface::GetVector2(direction), collider_filter,
			[](bool should_raycast, float fraction, float closest_fraction, SceneIndex, Entity) -> bool 
			{ 
				return should_raycast && fraction < closest_fraction; 
			});

	CSMonoObject cs_raycast_result(CSMonoCore::Get(), s_raycast_result_struct);

	CSMonoCore::Get()->SetValue(result.intersected, cs_raycast_result, s_intersected_field);
	if (result.intersected)
	{
		CSMonoCore::Get()->SetValue(GameObjectInterface::NewGameObjectWithExistingEntity(result.entity, result.scene_index),
			cs_raycast_result, s_intersected_game_object_field);
		CSMonoCore::Get()->SetValue(Vector2Interface::CreateVector2(result.position), cs_raycast_result, s_intersected_position_field);
	}

	return cs_raycast_result;
}

bool PhysicsInterface::RaycastCheckIfClosest(const CSMonoObject& position, const CSMonoObject& direction, uint16_t category, uint16_t mask, int16_t group, SceneIndex scene_index, Entity entity)
{
	const ColliderFilter collider_filter{ .category_bits = category, .mask_bits = mask, .group_index = group };
	const RaycastResult result =
		PhysicsCore::Get()->Raycast(Vector2Interface::GetVector2(position), Vector2Interface::GetVector2(direction), collider_filter,
			[scene_index, entity](bool should_raycast, float fraction, float closest_fraction, SceneIndex in_scene_index, Entity in_entity) -> bool {
				if (in_scene_index == scene_index && in_entity == entity)
				{
					should_raycast = true;
				}
				return should_raycast && fraction < closest_fraction;
			});

	return result.intersected && result.scene_index == scene_index && result.entity == entity;
}
