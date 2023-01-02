#pragma once
#include "DX12SwapChain.h"
#include "DX12CommandQueue.h"
#include "DX12DescriptorManager.h"
#include "DX12BufferManager.h"
#include "DX12TextureManager.h"
#include "DX12CommandList.h"
#include "DX12Fence.h"
#include "DX12ShaderCompiler.h"
#include "DX12ResourceDestroyer.h"

class Window;

class DX12Core
{
private:
	static constexpr uint8_t c_frames_in_flight = 2;

	Microsoft::WRL::ComPtr<IDXGIFactory5> m_factory;
	Microsoft::WRL::ComPtr<IDXGIAdapter1> m_adapter;
	Microsoft::WRL::ComPtr<ID3D12Device5> m_device;
	DX12CommandQueue m_graphics_command_queue;

	uint8_t m_commandlist_index = 0;
	DX12CommandList m_graphics_command_lists[c_frames_in_flight];

	DX12SwapChain m_swapchain;
	Window* m_window = nullptr;
	DX12DescriptorManager m_descriptor_manager;

	std::unique_ptr<DX12TextureManager> m_texture_manager;
	std::unique_ptr<DX12BufferManager> m_buffer_manager;

	DX12ShaderCompiler m_shader_compiler;

	DX12ResourceDestroyer m_resource_destroyer;

private:
	void EnableDebugLayer();
	void EnableGPUBasedValidation();
	void CreateFactory();
	void FindAdapter();
	void CreateDevice();
	void CreateCommandQueue();
	void CreateSwapChain(uint32_t backbuffer_count);
	void CreateDescriptorManager();
	void CreateTextureManager();
	void CreateBufferManager();
	void CreateCommandLists();

public:
	DX12Core() = default;
	void InitCore(Window* window, uint32_t backbuffer_count = 2);
	ID3D12Device5* GetDevice();
	DX12CommandQueue* GetGraphicsCommandQueue();
	Microsoft::WRL::ComPtr<IDXGIFactory5> GetFactory();
	Window* GetWindow();
	DX12TextureManager* GetTextureManager();
	DX12BufferManager* GetBufferManager();
	DX12DescriptorManager* GetDescriptorManager();
	DX12CommandList* GetCommandList();
	DX12SwapChain* GetSwapChain();
	IDXGIAdapter1* GetAdapter();
	void ResetBuffers();
	void EndOfFrame();
	static uint32_t GetFramesInFlight();
	uint32_t GetCurrentFrameInFlight() const;
	uint32_t GetCurrentBackBufferIndex() const;
	DX12ShaderCompiler* GetShaderCompiler();
	DX12ResourceDestroyer* GetResourceDestroyer();
};

