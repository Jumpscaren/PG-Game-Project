#pragma once
#include "ECS/EntityDefinition.h"
#include "Renderer/RenderTypes.h"
#include "Common/EngineTypes.h"
#include "Components/CameraComponent.h"

class DrawScene
{
private:
	struct BlockData
	{
		Entity block_entity;
	};

private:
	std::unordered_map<uint64_t, BlockData> m_blocks;

private:
	uint64_t GetNumberFromPosition(const Vector3& position);
	Vector3 GetWorldPositionFromMouse(const CameraComponent& editor_camera_component);

public:
	DrawScene();

	void Update();

	void Save();
	void Load();
};

