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
#include "AnimatorHandler.h"

class EntityManager;
class JsonObject;
class TileHandler;

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
	qr::unordered_map<uint64_t, qr::unordered_map<uint32_t, BlockData>> m_blocks;

	PrefabData m_prefab_selected = {};

	static qr::unordered_map<std::string, std::vector<PrefabAndTextureData>> m_user_prefabs;

	std::string m_scene_name;

	bool m_select;
	Entity m_select_entity;
	bool m_in_polygon_builder;

	std::vector<PolygonEntity> m_polygon_vertices;
	PolygonEntity m_polygon_vertex;
	bool m_switch_between;

	bool m_in_editor_menu;

	qr::unordered_map<std::string, uint8_t> m_names_already_in_use;

	static std::string m_category_in_use;

	static std::vector<Entity> m_entities_waiting_for_assets;

	TileHandler* m_tile_handler;

	bool m_in_animation;
	AnimatorHandler m_animator_handler;

	int32_t m_layer_index = 0;
	bool m_show_only_layer = false;

private:
	uint64_t GetNumberFromPosition(const Vector3& position) const;
	Vector3 GetWorldPositionFromMouse(const CameraComponent& editor_camera_component);
	BlockData CreateBlock(const Vector3& block_transform);
	void WriteData(JsonObject& json, const std::string& object_name);
	void ClearPolygonShaper();
	void AddPolygonEntity(const Vector2& point, const Vector3& select_position, const uint32_t index);

	void printtriangle(int i, int j);

	bool WaitForAssetsForPrefab();

public:
	DrawScene();

	void SetTileHandler(TileHandler* tile_handler) { m_tile_handler = tile_handler; }

	void Update();

	void DrawBlock();
	void ShowLayer();
	void Save(std::string scene_name);
	void Load(std::string scene_name);
	void Clear();
	void Select();
	void Animation();
	void PolygonShaper();

	bool InEditorMenu() const;

	const qr::unordered_map<uint32_t, BlockData>* GetBlocksFromPosition(const Vector3& position) const;

	static void SetAddUserPrefab();
	static void AddUserPrefab(const std::string& prefab_name, uint32_t z_index, const std::string& category);

	void SetSelectEntity(Entity entity) { m_select_entity = entity; }
};
