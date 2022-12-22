#include "pch.h"
#include "QREntryPoint.h"
#include "Renderer/RenderCore.h"

RenderCore* render_core;

void QREntryPoint::EntryPoint()
{
	render_core = new RenderCore(1920, 1080, L"2DRENDERER");
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
