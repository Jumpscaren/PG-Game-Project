#include "pch.h"
#include "QREntryPoint.h"
#include "Renderer/RenderCore.h"
#include "ECS/EntityManager.h"
#include "SceneSystem/SceneManager.h"

RenderCore* render_core;
SceneManager* scene_manager;
SceneIndex main_scene;

struct TempData
{
	TempData(int mm)
	{
		std::cout << "Hello\n";
		mine = mm * 2;
	}
	int mine;
};

struct TempComp
{
	int mine;
};

void QREntryPoint::EntryPoint()
{
	EntityManager ent(100);
	Entity e = ent.NewEntity();
	TempData& data = ent.AddComponent<TempData>(e, 2);
	++data.mine;

	bool check = ent.HasComponent<TempData>(e);
	check = ent.HasComponent<TempComp>(e);

	data = ent.GetComponent<TempData>(e);
	//Crashes as intended
	//ent.GetComponent<TempComp>(e);

	ent.System<TempData>([&](TempData& mdata)
		{
			std::cout << mdata.mine << "\n";
		});

	ent.System<TempData>([&](Entity e, TempData& mdata)
		{
			std::cout << e << " GG " << mdata.mine << "\n";
		});

	ent.AddComponent<TempComp>(e);
	ent.RemoveComponent<TempComp>(e);
	ent.AddComponent<TempComp>(e);

	ent.RemoveEntity(e);

	std::cout << "h\n";

	render_core = new RenderCore(1920, 1080, L"2DRENDERER");

	scene_manager = new SceneManager();
	main_scene = scene_manager->CreateScene();
	scene_manager->SetSceneAsActiveScene(main_scene);
}

void QREntryPoint::RunTime()
{
	bool window_exist = true;
	while (window_exist)
	{
		window_exist = render_core->UpdateRender(scene_manager->GetScene(main_scene));
		if (!window_exist)
			break;
	}
}
