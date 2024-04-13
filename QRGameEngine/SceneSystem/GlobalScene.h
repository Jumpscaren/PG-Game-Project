#pragma once
#include "SceneDefines.h"

class GlobalScene
{
private:
	SceneIndex m_scene_index;
	static GlobalScene* s_global_scene;

public:
	GlobalScene();

	static GlobalScene* Get();

	const SceneIndex GetSceneIndex();
};

