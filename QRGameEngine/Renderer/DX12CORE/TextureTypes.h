#pragma once
#include "pch.h"
#include "ViewTypes.h"
#include "HelpTypes.h"
#include "DX12MemAllocInclude.h"
#include "DescriptorTypes.h"

struct DX12Texture
{
	Microsoft::WRL::ComPtr<ID3D12Resource> texture_resource;
	Microsoft::WRL::ComPtr<D3D12MA::Allocation> texture_allocation;
};

typedef uint64_t DX12TextureHandle;

struct DX12TextureView
{
	DX12TextureHandle texture_handle;
	ViewType texture_view_type;
	DescriptorHandle texture_descriptor_handle;
};

typedef uint64_t DX12TextureViewHandle;

enum class TextureFlags
{
	DEPTSTENCIL_DENYSHADER_FLAG = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL | D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE,
	RENDER_TARGET_FLAG = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET,
	UNORDERED_ACCESS_FLAG = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
	NONE_FLAG = D3D12_RESOURCE_FLAG_NONE,
};