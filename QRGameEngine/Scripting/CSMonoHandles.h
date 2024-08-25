#pragma once
#include "pch.h"

struct MonoClassHandle
{
	uint64_t handle;

	bool operator ==(const MonoClassHandle& other) const
	{
		return handle == other.handle;
	}
};

struct MonoFieldHandle
{
	uint64_t handle;
};

struct MonoMethodHandle
{
	uint64_t handle;

	bool operator ==(const MonoMethodHandle& other) const
	{
		return handle == other.handle;
	}
};

struct MonoObjectHandle
{
	uint64_t handle;
};