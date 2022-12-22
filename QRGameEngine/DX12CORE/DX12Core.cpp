#include "pch.h"
#include "DX12Core.h"
#include "Window.h"

extern "C" { __declspec(dllexport) extern const UINT D3D12SDKVersion = 608; }
extern "C" { __declspec(dllexport) extern const char* D3D12SDKPath = ".\\D3D12\\"; }

using Microsoft::WRL::ComPtr;

void DX12Core::EnableDebugLayer()
{
	ComPtr<ID3D12Debug> debugController;
	HRESULT hr = D3D12GetDebugInterface(IID_PPV_ARGS(debugController.GetAddressOf()));
	assert(SUCCEEDED(hr));
	debugController->EnableDebugLayer();
	debugController.Get()->Release();
}

void DX12Core::EnableGPUBasedValidation()
{
	ComPtr<ID3D12Debug1> debugController;
	HRESULT hr = D3D12GetDebugInterface(IID_PPV_ARGS(debugController.GetAddressOf()));
	assert(SUCCEEDED(hr));
	debugController->SetEnableGPUBasedValidation(true);
	debugController.Get()->Release();
}

void DX12Core::CreateFactory()
{
	//Create factory
	UINT factory_flags = 0;
	ComPtr<IDXGIFactory5> temp_factory;
	HRESULT hr = CreateDXGIFactory2(factory_flags, IID_PPV_ARGS(temp_factory.GetAddressOf()));
	assert(SUCCEEDED(hr));
	hr = temp_factory->QueryInterface(__uuidof(IDXGIFactory5), (void**)(m_factory.GetAddressOf()));
	assert(SUCCEEDED(hr));
}

void DX12Core::FindAdapter()
{
	m_adapter = nullptr;
	IDXGIAdapter1* temp_adapter = nullptr;
	UINT adapter_index = 0;

	HRESULT hr = 0;

	while (hr != DXGI_ERROR_NOT_FOUND)
	{
		hr = m_factory->EnumAdapters1(adapter_index, &temp_adapter);
		//Wait until we can not find a adapter
		if (FAILED(hr))
			continue;

		DXGI_ADAPTER_DESC1 adapter_desc;
		assert(temp_adapter);
		temp_adapter->GetDesc1(&adapter_desc);

		hr = D3D12CreateDevice(temp_adapter, D3D_FEATURE_LEVEL_12_0, __uuidof(ID3D12Device), nullptr);

		if (FAILED(hr))
		{
			temp_adapter->Release();
			temp_adapter = nullptr;
		}
		else
		{
			if (!m_adapter)
			{
				IDXGIAdapter1** main_adapter_double_ptr = m_adapter.GetAddressOf();
				*main_adapter_double_ptr = temp_adapter;
			}
			//Compare which of the adapters has the most vram and take the one with the most
			else
			{
				DXGI_ADAPTER_DESC1 temp_adapter_desc;
				DXGI_ADAPTER_DESC1 adapter_desc;
				m_adapter->GetDesc1(&adapter_desc);
				temp_adapter->GetDesc1(&temp_adapter_desc);

				if (adapter_desc.DedicatedVideoMemory < temp_adapter_desc.DedicatedVideoMemory)
				{
					//m_adapter.Get()->Release();
					m_adapter = temp_adapter;
				}
				else
				{
					temp_adapter->Release();
					temp_adapter = nullptr;
				}
			}
		}
		++adapter_index;
	}
}

void DX12Core::CreateDevice()
{
	ID3D12Device* temp_device = nullptr;
	HRESULT hr = D3D12CreateDevice(m_adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&temp_device));
	assert(SUCCEEDED(hr));
	hr = temp_device->QueryInterface(__uuidof(ID3D12Device5),
		reinterpret_cast<void**>(m_device.GetAddressOf()));
	assert(SUCCEEDED(hr));
	temp_device->Release();
}

void DX12Core::CreateCommandQueue()
{
	m_graphics_command_queue.InitCommandQueue(this, CommandListType::GRAPHICS);
}

void DX12Core::CreateSwapChain(uint32_t backbuffer_count)
{
	m_swapchain.InitSwapChain(this, backbuffer_count);
}

void DX12Core::CreateDescriptorManager()
{
	m_descriptor_manager.InitDescriptorManager(this);
}

void DX12Core::CreateTextureManager()
{
	m_texture_manager = std::make_unique<DX12TextureManager>(this);
}

void DX12Core::CreateBufferManager()
{
	m_buffer_manager = std::make_unique<DX12BufferManager>(this);
}

void DX12Core::CreateCommandLists()
{
	for (uint32_t i = 0; i < c_frames_in_flight; ++i)
	{
		m_graphics_command_lists[i].InitCommandList(this, CommandListType::GRAPHICS);
	}
}

void DX12Core::InitCore(Window* window, uint32_t backbuffer_count)
{
	m_window = window;

	if (backbuffer_count < 2)
		assert(false);

#ifdef _DEBUG
	EnableDebugLayer();
	EnableGPUBasedValidation();
#endif // DEBUG

	CreateFactory();
	FindAdapter();
	CreateDevice();
	CreateDescriptorManager();
	CreateTextureManager();
	CreateBufferManager();
	CreateCommandQueue();
	CreateCommandLists();
	CreateSwapChain(backbuffer_count);
}

ID3D12Device5* DX12Core::GetDevice()
{
	return m_device.Get();
}

DX12CommandQueue* DX12Core::GetGraphicsCommandQueue()
{
	return &m_graphics_command_queue;
}

Microsoft::WRL::ComPtr<IDXGIFactory5> DX12Core::GetFactory()
{
	return m_factory;
}

Window* DX12Core::GetWindow()
{
	return m_window;
}

DX12TextureManager* DX12Core::GetTextureManager()
{
	return m_texture_manager.get();
}

DX12BufferManager* DX12Core::GetBufferManager()
{
	return m_buffer_manager.get();
}

DX12DescriptorManager* DX12Core::GetDescriptorManager()
{
	return &m_descriptor_manager;
}

DX12CommandList* DX12Core::GetCommandList()
{
	return &(m_graphics_command_lists[m_commandlist_index]);
}

DX12SwapChain* DX12Core::GetSwapChain()
{
	return &m_swapchain;
}

IDXGIAdapter1* DX12Core::GetAdapter()
{
	return m_adapter.Get();
}

void DX12Core::ResetBuffers()
{
	m_buffer_manager->ResetUploadBuffer(this);
	m_texture_manager->ResetUploadBuffer();
}

void DX12Core::EndOfFrame()
{
	ResetBuffers();

	m_commandlist_index = m_swapchain.GetCurrentBackBufferIndex() % c_frames_in_flight;
}

uint32_t DX12Core::GetFramesInFlight()
{
	return (uint32_t)c_frames_in_flight;
}

uint32_t DX12Core::GetCurrentBackBufferIndex() const
{
	return m_swapchain.GetCurrentBackBufferIndex();
}

DX12ShaderCompiler* DX12Core::GetShaderCompiler()
{
	return &m_shader_compiler;
}

DX12ResourceDestroyer* DX12Core::GetResourceDestroyer()
{
	return &m_resource_destroyer;
}
