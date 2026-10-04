#pragma once
#include "Renderer/DX12CORE/DX12RootSignature.h"
#include "Renderer/DX12CORE/DX12Pipeline.h"
#include "Interfaces/IMaterial.h"

class DX12Core;

class Material : public IMaterial
{
	friend class MaterialDatabase;
public:
	Material() = default;
	~Material() override = default;

	void SetVertexShader(const std::wstring& vertex_shader_path) override { m_vertex_shader_path = vertex_shader_path; }
	void SetPixelShader(const std::wstring& pixel_shader_path) override { m_pixel_shader_path = pixel_shader_path; }

	DX12RootSignature* getRootSignature() { return &m_root_signature; }
	DX12Pipeline* getPipeline() { return &m_pipeline; }

private:
	DX12RootSignature m_root_signature;
	DX12Pipeline m_pipeline;

	std::wstring m_vertex_shader_path = L"../QRGameEngine/Shaders/VertexShader.hlsl";
	std::wstring m_pixel_shader_path = L"../QRGameEngine/Shaders/PixelShader.hlsl";

private:
	void Initialize(DX12Core* dx12_core);
};
