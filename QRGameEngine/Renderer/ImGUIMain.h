#pragma once
#include "Vendor/Include/ImGUI/imgui.h"
#include "Vendor/Include/ImGUI/backends/imgui_impl_win32.h"
#include "Vendor/Include/ImGUI/backends/imgui_impl_dx12.h"
#include "RenderTypes.h"

class DX12Core;

class ImGUIMain
{
private:
	ImGUIMain();
	~ImGUIMain();

public:
	ImGUIMain(const ImGUIMain& other) = delete;
	ImGUIMain& operator=(const ImGUIMain& other) = delete;

	static void Init(DX12Core* dx12_core);
	static void Destroy();

	static void StartFrame();
	static void EndFrame(DX12Core* dx12_core);

	static bool ImageButton(const std::string& image_id, TextureHandle image_texture);
};

