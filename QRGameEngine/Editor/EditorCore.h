#pragma once
#include "DrawScene.h"
#include "ECS/EntityManager.h"

class EditorCore
{
private:
	static EditorCore* s_editor_core;

	Entity m_editor_camera_ent;

	DrawScene m_draw_scene;

public:
	EditorCore();

	static EditorCore* Get();

	void Update();

	Entity GetEditorCameraEntity() const;
};

