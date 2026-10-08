#pragma once
#include "pch.h"
#include "Renderer/DX12Core/RootSignatureTypes.h"
#include "Renderer/DX12Core/ViewTypes.h"
#include "Renderer/DX12Core/HelpTypes.h"

namespace material_types {
	struct MaterialIndex
	{
		uint32_t index = 0;

		std::strong_ordering operator<=>(const MaterialIndex& other) const = default;
	};

	constexpr MaterialIndex NULL_MATERIAL_INDEX = MaterialIndex{ .index = static_cast<uint32_t>(-1) };

	struct MaterialParameterIndex
	{
		std::size_t index = 0;

		std::strong_ordering operator<=>(const MaterialParameterIndex& other) const = default;
	};

	using MaterialParameterDataValue = std::variant<uint32_t, DX12TextureViewHandle, DX12BufferViewHandle, float>;
	struct MaterialParameterData {
		root_signature_types::RootParameterIndex root_parameter_index{};
		MaterialParameterDataValue value;

		ShaderVisibility shader_visibility;
		uint32_t shader_binding_index;
		uint32_t shader_space;
	};

	namespace detail {
		template <typename ...>
		struct IsVariantAlternative : std::false_type {};

		template <typename T, typename... Args>
		struct IsVariantAlternative <T, std::variant<Args...>> : std::bool_constant<(std::is_same_v<T, Args> || ...)> {};

		template <typename... Ts>
		struct Overloaded : Ts... { using Ts::operator()...; };
	}

	template <typename T>
	concept IsAllowedMaterialParameterType = detail::IsVariantAlternative<T, MaterialParameterDataValue>::value;
}