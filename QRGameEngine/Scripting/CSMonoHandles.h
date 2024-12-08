#pragma once
#include "pch.h"

struct MonoClassHandle
{
	uint64_t handle;

	bool operator ==(const MonoClassHandle& other) const
	{
		return handle == other.handle;
	}

	operator uint64_t() const
	{
		return handle;
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

struct MonoThreadHandle
{
	uint64_t handle;

	bool operator ==(const MonoThreadHandle other) const
	{
		return handle == other.handle;
	}

	operator uint64_t() const
	{
		return handle;
	}
};