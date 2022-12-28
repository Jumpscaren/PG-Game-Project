#include "pch.h"
#include "DX12Pipeline.h"
#include "DX12RootSignature.h"
#include "DX12Core.h"
#include "../../Vendor/Include/D3D12ShaderCompiler/dxcapi.h"

ID3DBlob* DX12Pipeline::LoadCSO(const std::string& shader_name)
{
	std::ifstream file(shader_name, std::ios::binary);
	if (!file.is_open())
		assert(false);
	file.seekg(0, std::ios_base::end);
	size_t size = static_cast<size_t>(file.tellg());
	file.seekg(0, std::ios_base::beg);
	ID3DBlob* cso_blob = nullptr;
	HRESULT hr = D3DCreateBlob(size, &cso_blob);
	if (FAILED(hr))
		assert(false);
	file.read(static_cast<char*>(cso_blob->GetBufferPointer()), size);
	file.close();
	return cso_blob;
}

DX12Pipeline::~DX12Pipeline()
{
}

void DX12Pipeline::InitPipeline(DX12Core* dx12_core, DX12RootSignature* root_signature, const std::wstring& vertex_shader, const std::wstring& pixel_shader)
{
	//Create pipeline
	D3D12_GRAPHICS_PIPELINE_STATE_DESC pipeline_desc;
	ZeroMemory(&pipeline_desc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));

	pipeline_desc.pRootSignature = root_signature->GetRootSignature();

	D3D12_SHADER_BYTECODE* vs_shader = &pipeline_desc.VS;
	D3D12_SHADER_BYTECODE* ps_shader = &pipeline_desc.PS;

	CompiledShader compiled_vertex_shader = dx12_core->GetShaderCompiler()->CompileShader(vertex_shader, ShaderType::VERTEX);
	CompiledShader compiled_pixel_shader = dx12_core->GetShaderCompiler()->CompileShader(pixel_shader, ShaderType::PIXEL);

	vs_shader->pShaderBytecode = compiled_vertex_shader.blob->GetBufferPointer();
	vs_shader->BytecodeLength = compiled_vertex_shader.blob->GetBufferSize();
	ps_shader->pShaderBytecode = compiled_pixel_shader.blob->GetBufferPointer();
	ps_shader->BytecodeLength = compiled_pixel_shader.blob->GetBufferSize();

	pipeline_desc.SampleMask = UINT_MAX;

	D3D12_RASTERIZER_DESC rasterizer_desc;
	rasterizer_desc.FillMode = D3D12_FILL_MODE_SOLID;
	rasterizer_desc.CullMode = D3D12_CULL_MODE_BACK;
	rasterizer_desc.FrontCounterClockwise = false;
	rasterizer_desc.DepthBias = 0;
	rasterizer_desc.DepthBiasClamp = 0.0f;
	rasterizer_desc.SlopeScaledDepthBias = 0.0f;
	rasterizer_desc.DepthClipEnable = true;
	rasterizer_desc.MultisampleEnable = false;
	rasterizer_desc.AntialiasedLineEnable = false;
	rasterizer_desc.ForcedSampleCount = 0;
	rasterizer_desc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
	pipeline_desc.RasterizerState = rasterizer_desc;

	pipeline_desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	pipeline_desc.NumRenderTargets = static_cast<UINT>(1);

	pipeline_desc.BlendState.AlphaToCoverageEnable = false;
	pipeline_desc.BlendState.IndependentBlendEnable = false;

	pipeline_desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	D3D12_RENDER_TARGET_BLEND_DESC rt_blend_desc;
	rt_blend_desc.BlendEnable = false;
	rt_blend_desc.LogicOpEnable = false;
	rt_blend_desc.SrcBlend = D3D12_BLEND_ONE;
	rt_blend_desc.DestBlend = D3D12_BLEND_ZERO;
	rt_blend_desc.BlendOp = D3D12_BLEND_OP_ADD;
	rt_blend_desc.SrcBlendAlpha = D3D12_BLEND_ONE;
	rt_blend_desc.DestBlendAlpha = D3D12_BLEND_ZERO;
	rt_blend_desc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
	rt_blend_desc.LogicOp = D3D12_LOGIC_OP_NOOP;
	rt_blend_desc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	pipeline_desc.BlendState.RenderTarget[0] = rt_blend_desc;

	pipeline_desc.SampleDesc.Count = 1;
	pipeline_desc.SampleDesc.Quality = 0;

	D3D12_DEPTH_STENCIL_DESC depth_stencil_desc;
	depth_stencil_desc.DepthEnable = true;
	depth_stencil_desc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	depth_stencil_desc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	depth_stencil_desc.StencilEnable = false;
	depth_stencil_desc.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
	depth_stencil_desc.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
	depth_stencil_desc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	depth_stencil_desc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	depth_stencil_desc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	depth_stencil_desc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	depth_stencil_desc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	depth_stencil_desc.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	depth_stencil_desc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	depth_stencil_desc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;

	D3D12_STREAM_OUTPUT_DESC stream_output_desc;
	stream_output_desc.pSODeclaration = nullptr;
	stream_output_desc.NumEntries = 0;
	stream_output_desc.pBufferStrides = nullptr;
	stream_output_desc.NumStrides = 0;
	stream_output_desc.RasterizedStream = 0;

	pipeline_desc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	pipeline_desc.DepthStencilState = depth_stencil_desc;
	pipeline_desc.StreamOutput = stream_output_desc;
	pipeline_desc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

	HRESULT hr = dx12_core->GetDevice()->CreateGraphicsPipelineState(&pipeline_desc, IID_PPV_ARGS(m_pipeline_state.GetAddressOf()));
	assert(SUCCEEDED(hr));

	root_signature->ClearParameters();
}

ID3D12PipelineState* DX12Pipeline::GetPipeline()
{
	return m_pipeline_state.Get();
}
