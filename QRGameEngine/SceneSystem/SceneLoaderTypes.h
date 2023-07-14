#pragma once
#include "pch.h"
#include "ECS/EntityDefinition.h"

struct PrefabData
{
	uint32_t prefab_index;
	uint32_t z_index;
};

struct BlockData
{
	Entity block_entity;
	PrefabData prefab_data;
};