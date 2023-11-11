#pragma once
#include "ECS/EntityDefinition.h"
#include "Renderer/RenderTypes.h"
#include "Common/EngineTypes.h"
#include "Components/CameraComponent.h"
#include "Renderer/RenderTypes.h"
#include "IO/Output.h"
#include <functional>
#include "SceneSystem/SceneLoaderTypes.h"

class EntityManager;


struct PrefabAndTextureData
{
	PrefabData prefab_data;
	TextureHandle texture_handle;
};

class DrawScene
{
private:
	std::unordered_map<uint64_t, std::unordered_map<uint32_t, BlockData>> m_blocks;

	PrefabData m_prefab_selected = {};

	static std::vector<PrefabAndTextureData> m_user_prefabs;

	std::string m_scene_name;

	bool m_select;
	Entity m_select_entity;

	bool m_in_editor_menu;

private:
	uint64_t GetNumberFromPosition(const Vector3& position);
	Vector3 GetWorldPositionFromMouse(const CameraComponent& editor_camera_component);
	BlockData CreateBlock(const Vector3& block_transform);

public:
	DrawScene();

	void Update();

	void DrawBlock();
	void Save(std::string scene_name);
	void Load(std::string scene_name);
	void Clear();
	void Select();

	bool InEditorMenu() const;

	static void SetAddUserPrefab();
	static void AddUserPrefab(uint32_t prefab_instance_id, uint32_t z_index);
};
