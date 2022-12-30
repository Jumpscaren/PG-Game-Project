#pragma once

class EntityManager;

class Scene
{
private:
	std::unique_ptr<EntityManager> m_entity_manager;

public:
	Scene();
	~Scene();

	EntityManager* GetEntityManager();
};

