#pragma once
#include "pch.h"

struct MaterialIndex
{
	uint32_t index = 0;

	std::strong_ordering operator<=>(const MaterialIndex& other) const = default;
};

constexpr MaterialIndex NULL_MATERIAL_INDEX = MaterialIndex{ .index = static_cast<uint32_t>(-1) };