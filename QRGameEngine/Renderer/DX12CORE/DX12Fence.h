#pragma once
class DX12Core;
class DX12CommandQueue;

class DX12Fence
{
private:
	Microsoft::WRL::ComPtr<ID3D12Fence> m_fence;
	static constexpr uint64_t c_initial_value = 0;
	uint64_t m_fence_value;
	bool m_succeded = false;

public:
	DX12Fence();
	~DX12Fence();
	void InitFence(DX12Core* dx12_core);
	void Signal(DX12Core* dx12_core, DX12CommandQueue* command_queue);
	bool IsFenceCompleted(DX12Core* dx12_core);
	void Wait(DX12Core* dx12_core);
	void SignalAndWait(DX12Core* dx12_core, DX12CommandQueue* command_queue);
};

