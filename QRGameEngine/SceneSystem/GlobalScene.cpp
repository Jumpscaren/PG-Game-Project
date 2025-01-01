#include "pch.h"
#include "GlobalScene.h"
#include "SceneManager.h"

GlobalScene* GlobalScene::s_global_scene = nullptr;

GlobalScene::GlobalScene()
{
	s_global_scene = this;
	m_scene_index = SceneManager::GetSceneManager()->CreateScene();
	SceneManager::GetSceneManager()->GetScene(m_scene_index)->SetSceneAsLoaded();
	SceneManager::GetSceneManager()->GetScene(m_scene_index)->SetSceneAsActive();
}

GlobalScene* GlobalScene::Get()
{
	return s_global_scene;
}

const SceneIndex GlobalScene::GetSceneIndex()
{
	return m_scene_index;
}
