#include "pch.h"
#include "QREntryPoint.h"
#include "Renderer/RenderCore.h"
#include "ECS/EntityManager.h"

RenderCore* render_core;

struct TempData
{
	TempData(int mm)
	{
		std::cout << "Hello\n";
		mine = mm * 2;
	}
	int mine;
};

void QREntryPoint::EntryPoint()
{
	render_core = new RenderCore(1920, 1080, L"2DRENDERER");

	EntityManager ent(100);
	Entity e = ent.NewEntity();
	TempData& data = ent.AddComponent<TempData>(e, 2);
	++data.mine;
	std::cout << "h\n";
}

void QREntryPoint::RunTime()
{
	bool window_exist = true;
	while (window_exist)
	{
		window_exist = render_core->UpdateRender();
		if (!window_exist)
			break;
	}
}
