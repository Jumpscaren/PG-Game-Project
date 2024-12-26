#pragma once
#include "pch.h"
#include "ECS/EntityDefinition.h"
#include "SceneSystem/SceneDefines.h"
#include "Common/EngineTypes.h"

typedef uint64_t PhysicObjectHandle;
static constexpr PhysicObjectHandle NULL_PHYSIC_OBJECT_HANDLE = -1;

struct ColliderFilter
{
	uint16_t category_bits = 0x0001;
	uint16_t mask_bits = 0xFFFF;
	int16_t group_index = 0;
};

struct RaycastResult
{
	SceneIndex scene_index;
	Entity entity;
	Vector2 position;
	bool intersected;
};