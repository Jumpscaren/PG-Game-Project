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

struct PolygonEntity
{
	Entity ent;
	uint32_t vertex_index;
};

class DrawScene
{
private:
	std::unordered_map<uint64_t, std::unordered_map<uint32_t, BlockData>> m_blocks;

	PrefabData m_prefab_selected = {};

	static std::unordered_map<std::string, std::vector<PrefabAndTextureData>> m_user_prefabs;

	std::string m_scene_name;

	bool m_select;
	Entity m_select_entity;
	bool m_in_polygon_builder;

	std::vector<PolygonEntity> m_polygon_vertices;
	PolygonEntity m_polygon_vertex;
	bool m_switch_between;

	bool m_in_animation;
	const std::string m_animation_temp_save_file_name = "animation_temp_save_file_name";
	Entity m_animation_base_entity = NULL_ENTITY;
	std::string m_animation_texture_name;
	std::string m_animation_file_name;

	bool m_in_editor_menu;

	std::unordered_map<std::string, uint8_t> m_names_already_in_use;

	static std::string m_category_in_use;

private:
	uint64_t GetNumberFromPosition(const Vector3& position);
	Vector3 GetWorldPositionFromMouse(const CameraComponent& editor_camera_component);
	BlockData CreateBlock(const Vector3& block_transform);
	void WriteData(JsonObject& json, const std::string& object_name);
	void ClearPolygonShaper();
	void AddPolygonEntity(const Vector2& point, const Vector3& select_position, const uint32_t index);

	void printtriangle(int i, int j);

public:
	DrawScene();

	void Update();

	void DrawBlock();
	void Save(std::string scene_name);
	void Load(std::string scene_name);
	void Clear();
	void Select();
	void Animation();
	void PolygonShaper();

	bool InEditorMenu() const;

	static void SetAddUserPrefab();
	static void AddUserPrefab(const std::string& prefab_name, uint32_t z_index, const std::string& category);
};
