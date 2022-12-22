#include "Renderer/RenderCore.h"

int main()
{
	RenderCore render_core(1920, 1080, L"2DRENDERER");

	bool window_exist = true;
	while (window_exist)
	{
		window_exist = render_core.UpdateRender();
		if (!window_exist)
			break;
	}

	return 0;
}