#include "pch.h"
#include "ImGUIMain.h"
#include "DX12CORE/DX12Core.h"
#include "Window.h"
#include "RenderCore.h"

DX12Core* ImGUIMain::m_dx12_core = nullptr;

ImGUIMain::ImGUIMain()
{

}

ImGUIMain::~ImGUIMain()
{

}

ImVec2 ImGUIMain::GetScaledMousePosition(const Vector2u& backbuffer_size)
{
	POINT mouse_pos;
	GetCursorPos(&mouse_pos);
	ScreenToClient(m_dx12_core->GetWindow()->GetWindowHandle(), &mouse_pos);

	RECT rect;
	GetClientRect(m_dx12_core->GetWindow()->GetWindowHandle(), &rect);
	float window_width = float(rect.right - rect.left);
	float window_height = float(rect.bottom - rect.top);
	float scale_x = backbuffer_size.x / window_width;
	float scale_y = backbuffer_size.y / window_height;

	float imgui_mouse_x = mouse_pos.x * scale_x;
	float imgui_mouse_y = mouse_pos.y * scale_y;

	return ImVec2(imgui_mouse_x, imgui_mouse_y);
}

void ImGUIMain::Init(DX12Core* dx12_core)
{
	m_dx12_core = dx12_core;

	// Setup ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	ImGui::StyleColorsDark();

	auto descriptor_chunk = dx12_core->GetDescriptorManager()->GetDescriptorChunk(dx12_core, DescriptorHeapTypes::SHADERBINDABLE_VIEW, dx12_core->GetFramesInFlight());
	auto descriptor = descriptor_chunk.AddDescriptor();
	ImGui_ImplWin32_Init((void*)dx12_core->GetWindow()->GetWindowHandle());

	ImGui_ImplDX12_Init(dx12_core->GetDevice(), dx12_core->GetFramesInFlight(),
		DXGI_FORMAT_R8G8B8A8_UNORM, dx12_core->GetDescriptorManager()->GetShaderBindable()->GetDescriptorHeap(),
		descriptor.cpu_handle,
		descriptor.gpu_handle);

	ImGui::GetIO().FontAllowUserScaling = true;
}

void ImGUIMain::Destroy()
{
	// Cleanup
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void ImGUIMain::StartFrame()
{
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();

	//const Vector2u backbuffer_size = RenderCore::Get()->GetBackbufferSize();

	//ImGui::GetIO().MousePos = GetScaledMousePosition(backbuffer_size);

	//ImGui::GetIO().DisplaySize = ImVec2((float)backbuffer_size.x, (float)backbuffer_size.y);

	ImGui::NewFrame();
}

void ImGUIMain::RenderFrame(DX12Core* dx12_core)
{
	ImGui::Render();
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), dx12_core->GetCommandList()->GetCommandList());
}

void ImGUIMain::EndFrame()
{
	ImGui::EndFrame();
}

bool ImGUIMain::ImageButton(const std::string& image_id, const TextureHandle image_texture)
{
	const DX12TextureViewHandle texture_view_handle = RenderCore::Get()->GetTextureViewHandle(image_texture);
	return ImGui::ImageButton(image_id.c_str(), (ImTextureID)RenderCore::Get()->GetDX12Core()->GetTextureManager()->GetTextureView(texture_view_handle)->texture_descriptor_handle.gpu_handle.ptr, {100.0f, 100.0f});
}
