#pragma once
#include "pch.h"
#include "ECS/EntityDefinition.h"

struct PrefabData
{
	std::string prefab_name;
	uint32_t z_index;
};

struct BlockData
{
	Entity block_entity;
	PrefabData prefab_data;
};