#include "pch.h"
#include "DX12CommandQueue.h"
#include "DX12Core.h"

void DX12CommandQueue::InitCommandQueue(DX12Core* dx12_core, CommandListType command_type)
{
	D3D12_COMMAND_QUEUE_DESC desc;
	desc.Type = (D3D12_COMMAND_LIST_TYPE)command_type;
	desc.Priority = 0;
	desc.NodeMask = 0;
	desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	HRESULT hr = dx12_core->GetDevice()->CreateCommandQueue(&desc, IID_PPV_ARGS(m_command_queue.GetAddressOf()));
	assert(SUCCEEDED(hr));
}

void DX12CommandQueue::ExecuteCommandList(DX12Core* dx12_core, DX12CommandList* command_list)
{
	ID3D12CommandList* temp = command_list->GetCommandList();
	m_command_queue->ExecuteCommandLists(1, &temp);
}

void DX12CommandQueue::SignalCommandQueue(ID3D12Fence* fence, uint64_t fence_value)
{
	m_command_queue->Signal(fence, fence_value);
}

ID3D12CommandQueue* DX12CommandQueue::GetCommandQueue()
{
	return m_command_queue.Get();
}
