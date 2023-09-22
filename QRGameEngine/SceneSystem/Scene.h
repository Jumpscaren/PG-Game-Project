#pragma once
#include "SceneDefines.h"

class EntityManager;

class Scene
{
private:
	std::unique_ptr<EntityManager> m_entity_manager;
	SceneIndex m_scene_index;

public:
	Scene(SceneIndex scene_index);
	~Scene();

	EntityManager* GetEntityManager();
};

