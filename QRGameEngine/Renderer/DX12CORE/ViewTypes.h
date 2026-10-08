#pragma once
enum class ViewType
{
	RENDER_TARGET_VIEW = 1,
	SHADER_RESOURCE_VIEW = 2,
	DEPTH_STENCIL_VIEW = 4,
	UNORDERED_ACCESS_VIEW = 8,
	CONSTANT_BUFFER_VIEW = 16,
};

struct DX12BufferViewHandle
{
	uint64_t handle;

	std::strong_ordering operator<=>(const DX12BufferViewHandle& other) const = default;
};

struct DX12TextureViewHandle
{
	uint64_t handle;

	std::strong_ordering operator<=>(const DX12TextureViewHandle& other) const = default;
};
