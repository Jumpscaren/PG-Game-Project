#pragma once
class DX12Core;
class DX12RootSignature;
class DX12CommandList;

class DX12Pipeline
{
	friend DX12CommandList;

private:
	Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipeline_state;

private:
	ID3DBlob* LoadCSO(const std::string& shader_name);
	ID3D12PipelineState* GetPipeline();

public:
	DX12Pipeline() = default;
	~DX12Pipeline();

	void InitPipeline(DX12Core* dx12_core, DX12RootSignature* root_signature, const std::wstring& vertex_shader, const std::wstring& pixel_shader);
};

