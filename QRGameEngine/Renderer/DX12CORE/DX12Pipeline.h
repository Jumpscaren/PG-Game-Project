#pragma once
class DX12Core;
class DX12RootSignature;
class DX12CommandList;

class DX12Pipeline
{
	friend DX12CommandList;

private:
	Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipeline_state;

	struct PipelineInfo
	{
		D3D12_PRIMITIVE_TOPOLOGY_TYPE topology_type = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		bool depth_stencil = true;
		//More stuff if needed
	} m_pipeline_info;

private:
	ID3DBlob* LoadCSO(const std::string& shader_name);
	ID3D12PipelineState* GetPipeline();

public:
	DX12Pipeline() = default;
	~DX12Pipeline();

	DX12Pipeline& AddTopology(const D3D12_PRIMITIVE_TOPOLOGY_TYPE& topology_type);
	DX12Pipeline& AddDepthStencil(bool depth_stencil);
	void InitPipeline(DX12Core* dx12_core, DX12RootSignature* root_signature, const std::wstring& vertex_shader, const std::wstring& pixel_shader);
};

