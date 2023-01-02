#include "pch.h"
#include "QREntryPoint.h"
#include "Renderer/RenderCore.h"
#include "ECS/EntityManager.h"
#include "SceneSystem/SceneManager.h"
#include "EngineComponents.h"
#include "Time/Time.h"

RenderCore* render_core;
SceneManager* scene_manager;
SceneIndex main_scene;
Entity render_ent;

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

	TextureHandle texture = render_core->CreateTexture("../QRGameEngine/Textures/Temp.png");

	EntityManager* em = scene_manager->GetScene(main_scene)->GetEntityManager();
	render_ent = em->NewEntity();
	em->AddComponent<TransformComponent>(render_ent, Vector3(), Vector3(0.0f, 0.0f, DirectX::XM_PIDIV4 / 2.0f), Vector3(0.2f, 0.2f, 0.2f));
	em->AddComponent<SpriteComponent>(render_ent).texture_handle = texture;
}

void QREntryPoint::RunTime()
{
	bool window_exist = true;
	while (window_exist)
	{
		Time::Start();

		Scene* active_scene = scene_manager->GetScene(main_scene);
		EntityManager* entman = active_scene->GetEntityManager();

		TransformComponent& render_ent_trans = entman->GetComponent<TransformComponent>(render_ent);
		
		Vector3 pos = render_ent_trans.GetPosition();
		pos.x += (float)Time::GetDeltaTime();
		render_ent_trans.SetPosition(pos);


		window_exist = render_core->UpdateRender(scene_manager->GetScene(main_scene));
		if (!window_exist)
			break;

		Time::Stop();
	}

	delete render_core;
	delete scene_manager;
}
