#pragma once
#include "pch.h"

//using NodeIndex = uint32_t;
struct NodeIndex
{
	uint32_t value;

	void increase() { ++value; }

	bool operator==(const NodeIndex& other) const { return value == other.value; }
	bool operator!=(const NodeIndex& other) const { return value != other.value; }
};
static constexpr NodeIndex NULL_NODE_INDEX = NodeIndex{.value = (uint32_t)-1};

struct NodeIndexHasher
{
	std::size_t operator()(const NodeIndex node_index) const
	{
		std::size_t res = 17;
		res = res * 31 + std::hash<uint32_t>()(node_index.value);
		return res;
	}
};

enum class NodeDetail
{
	Large = 0, // 0.25 to 1
	Base = 1, // 1 to 1
	Small = 2, // 4 to 1
};