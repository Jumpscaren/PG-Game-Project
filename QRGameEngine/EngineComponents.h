#pragma once
#include "Renderer/RenderTypes.h"
#include "Common/EngineTypes.h"

struct TransformComponent
{
	TransformComponent(const Vector3& position = {}, const Vector3& rotation = {}, const Vector3& scale = {});

	TransformComponent& SetPosition(const Vector3& position);
	TransformComponent& SetRotation(const Vector3& rotation);
	TransformComponent& SetScale(const Vector3& scale);

	Vector3 GetPosition() const;
	Vector4 GetRotation() const;
	Vector3 GetScale() const;

	DirectX::XMMATRIX world_matrix;
};

struct SpriteComponent
{
	TextureHandle texture_handle;
	struct UV
	{
		float x, y;
	} uv;
};