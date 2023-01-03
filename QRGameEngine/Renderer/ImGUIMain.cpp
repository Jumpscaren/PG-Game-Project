#include "pch.h"
#include "ImGUIMain.h"
#include "DX12CORE/DX12Core.h"
#include "Window.h"

ImGUIMain::ImGUIMain()
{

}

ImGUIMain::~ImGUIMain()
{

}

void ImGUIMain::Init(DX12Core* dx12_core)
{
	// Setup ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	ImGui::StyleColorsDark();

	auto descriptor_chunk = dx12_core->GetDescriptorManager()->GetDescriptorChunk(dx12_core, DescriptorHeapTypes::SHADERBINDABLE_VIEW, 2);
	auto descriptor = descriptor_chunk.AddDescriptor();
	ImGui_ImplWin32_Init((void*)dx12_core->GetWindow()->GetWindowHandle());

	ImGui_ImplDX12_Init(dx12_core->GetDevice(), 2,
		DXGI_FORMAT_R8G8B8A8_UNORM, dx12_core->GetDescriptorManager()->GetShaderBindable()->GetDescriptorHeap(),
		descriptor.cpu_handle,
		descriptor.gpu_handle);
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
	ImGui::NewFrame();
}

void ImGUIMain::EndFrame(DX12Core* dx12_core)
{
	ImGui::Render();
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), dx12_core->GetCommandList()->GetCommandList());
}
