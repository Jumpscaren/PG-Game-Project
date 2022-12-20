#pragma once
#include "HelpTypes.h"

class DX12Core;
class DX12CommandList;

class DX12CommandQueue
{
private:
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_command_queue;

public:
	DX12CommandQueue() = default;
	void InitCommandQueue(DX12Core* dx12_core, CommandListType command_type);
	void ExecuteCommandList(DX12Core* dx12_core, DX12CommandList* command_list);
	void SignalCommandQueue(ID3D12Fence* fence, uint64_t fence_value);
	ID3D12CommandQueue* GetCommandQueue();
};

