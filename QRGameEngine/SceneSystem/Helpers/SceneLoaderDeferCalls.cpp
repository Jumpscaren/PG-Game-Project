#include "pch.h"
#include "SceneLoaderDeferCalls.h"
#include "SceneSystem/SceneLoader.h"

SceneLoaderDeferCalls::SceneLoaderDeferCalls() : m_semaphore(0)
{
}

bool SceneLoaderDeferCalls::ShouldCallDirectly(const SceneIndex current_scene_index) const
{
	return !SceneLoader::Get()->DeferMethodCall() || SceneLoader::Get()->GetLoadingScene() != current_scene_index;
}
