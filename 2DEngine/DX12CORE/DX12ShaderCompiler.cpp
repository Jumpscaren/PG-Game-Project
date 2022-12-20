#include "pch.h"
#include "DX12ShaderCompiler.h"
#include "../Vendor/Include/D3D12ShaderCompiler/dxcapi.h"

DX12ShaderCompiler::DX12ShaderCompiler()
{
	HRESULT hr = S_OK;

	hr = DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(&m_library));
	assert(SUCCEEDED(hr));

	hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&m_compiler));
	assert(SUCCEEDED(hr));

	hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&m_utils));
	assert(SUCCEEDED(hr));

	// Grab default include handler
	m_utils->CreateDefaultIncludeHandler(&m_defIncHandler);
}

DX12ShaderCompiler::~DX12ShaderCompiler()
{
}

CompiledShader DX12ShaderCompiler::CompileShader(const std::wstring& shader_path, const ShaderType& shader_type)
{
	uint32_t codePage = CP_UTF8;
	Microsoft::WRL::ComPtr<IDxcBlobEncoding> source_blob;

	HRESULT hr;
	hr = m_library->CreateBlobFromFile(shader_path.c_str(), &codePage, source_blob.GetAddressOf());
	assert(SUCCEEDED(hr));

	std::wstring profile;

	switch (shader_type)
	{
	case ShaderType::VERTEX:
		profile = L"vs_6_6";
		break;
	case ShaderType::PIXEL:
		profile = L"ps_6_6";
		break;
	}

	Microsoft::WRL::ComPtr<IDxcOperationResult> result;
	hr = m_compiler->Compile(
		source_blob.Get(), // pSource
		shader_path.c_str(),// pSourceName
		L"main", // pEntryPoint
		profile.c_str(), // pTargetProfile
		NULL, 0, // pArguments, argCount
		NULL, 0, // pDefines, defineCount
		m_defIncHandler.Get(),
		result.GetAddressOf()); // ppResult

	if (SUCCEEDED(hr))
		result->GetStatus(&hr);

	// Check error
	if (FAILED(hr))
	{
		Microsoft::WRL::ComPtr<IDxcBlobEncoding> errors;
		hr = result->GetErrorBuffer(&errors);
		if (SUCCEEDED(hr) && errors)
		{
			wprintf(L"Compilation failed with errors:\n%hs\n",
				(const char*)errors->GetBufferPointer());
			assert(false);
		}
	}

	Microsoft::WRL::ComPtr<IDxcBlob> res;
	hr = result->GetResult(res.GetAddressOf());
	if (FAILED(hr))
		assert(false);

	CompiledShader compiled_shader;
	compiled_shader.blob = res;

	return compiled_shader;
}
