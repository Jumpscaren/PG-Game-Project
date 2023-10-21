#pragma once
#include "SceneDefines.h"

class EntityManager;

class Scene
{
private:
	std::unique_ptr<EntityManager> m_entity_manager;
	SceneIndex m_scene_index;
	bool m_scene_loaded;

public:
	Scene(SceneIndex scene_index);
	~Scene();

	void SetSceneAsLoaded();
	bool IsSceneLoaded();

	EntityManager* GetEntityManager();
};

