#include "pch.h"
#include "DX12Fence.h"
#include "DX12Core.h"

DX12Fence::DX12Fence()
{
	m_fence_value = 0;
}

DX12Fence::~DX12Fence()
{
}

void DX12Fence::InitFence(DX12Core* dx12_core)
{
	HRESULT hr = dx12_core->GetDevice()->CreateFence(c_initial_value, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_fence.GetAddressOf()));
	assert(SUCCEEDED(hr));

	m_fence_value = c_initial_value;
	m_succeded = false;
}

void DX12Fence::Signal(DX12Core* dx12_core, DX12CommandQueue* command_queue)
{
	++m_fence_value;
	m_succeded = false;
	HRESULT hr = command_queue->GetCommandQueue()->Signal(m_fence.Get(), m_fence_value);
	assert(SUCCEEDED(hr));
}

void DX12Fence::Wait(DX12Core* dx12_core)
{
	//std::cout << "Succeded: " << m_succeded << "\n";

	if (!m_succeded && m_fence->GetCompletedValue() < m_fence_value)
	{
		HANDLE eventHandle = CreateEventEx(nullptr, 0, 0, EVENT_ALL_ACCESS);
		HRESULT hr = m_fence->SetEventOnCompletion(m_fence_value, eventHandle);
		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);

		m_succeded = true;
	}
}

void DX12Fence::SignalAndWait(DX12Core* dx12_core, DX12CommandQueue* command_queue)
{
	Signal(dx12_core, command_queue);
	Wait(dx12_core);
}
