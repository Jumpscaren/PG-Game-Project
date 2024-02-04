#pragma once
#include "ECS/EntityDefinition.h"
#include "Renderer/RenderTypes.h"
#include "Common/EngineTypes.h"
#include "Components/CameraComponent.h"
#include "Renderer/RenderTypes.h"
#include "IO/Output.h"
#include <functional>
#include "SceneSystem/SceneLoaderTypes.h"
#include "SceneSystem/SceneDefines.h"

class EntityManager;
class JsonObject;

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

	bool m_in_animation;
	const std::string m_animation_temp_save_file_name = "animation_temp_save_file_name";
	Entity m_animation_base_entity = NULL_ENTITY;
	std::string m_animation_texture_name;
	std::string m_animation_file_name;

	bool m_in_editor_menu;

	std::unordered_map<std::string, uint8_t> m_names_already_in_use;

private:
	uint64_t GetNumberFromPosition(const Vector3& position);
	Vector3 GetWorldPositionFromMouse(const CameraComponent& editor_camera_component);
	BlockData CreateBlock(const Vector3& block_transform);
	void WriteData(JsonObject& json, const std::string& object_name);

public:
	DrawScene();

	void Update();

	void DrawBlock();
	void Save(std::string scene_name);
	void Load(std::string scene_name);
	void Clear();
	void Select();
	void Animation();

	bool InEditorMenu() const;

	static void SetAddUserPrefab();
	static void AddUserPrefab(std::string prefab_name, uint32_t z_index);
};
