#pragma once
#include "SceneDefines.h"

class EntityManager;
class SceneManager;

class Scene
{
	friend SceneManager;

private:
	std::unique_ptr<EntityManager> m_entity_manager;
	SceneIndex m_scene_index;
	bool m_scene_loaded;
	bool m_scene_active;
	std::string m_scene_name;

private:
	void SetSceneName(const std::string& scene_name);

public:
	Scene(SceneIndex scene_index);
	~Scene();

	void SetSceneAsLoaded();
	bool IsSceneLoaded();

	void SetSceneAsActive();
	bool IsSceneActive();

	SceneIndex GetSceneIndex() const; 
	const std::string& GetSceneName() const;

	EntityManager* GetEntityManager();
};

