#pragma once
#include "DrawScene.h"
#include "ECS/EntityManager.h"
#include "TileHandler.h"

class EditorCore
{
private:
	static EditorCore* s_editor_core;

	Entity m_editor_camera_ent;

	DrawScene m_draw_scene;
	TileHandler m_tile_handler;

public:
	EditorCore();

	static EditorCore* Get();

	void Update();

	Entity GetEditorCameraEntity() const;
};

