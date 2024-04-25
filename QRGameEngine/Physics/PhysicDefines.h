#pragma once
#include "pch.h"

typedef uint64_t PhysicObjectHandle;
static constexpr PhysicObjectHandle NULL_PHYSIC_OBJECT_HANDLE = -1;

struct ColliderFilter
{
	uint16_t category_bits = 0x0001;
	uint16_t mask_bits = 0xFFFF;
	int16_t group_index = 0;
};