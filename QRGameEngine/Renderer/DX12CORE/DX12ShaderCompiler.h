#pragma once
#include "HelpTypes.h"

struct IDxcLibrary;
struct IDxcUtils;
struct IDxcCompiler;
struct IDxcIncludeHandler;
struct IDxcBlob;

struct CompiledShader
{
	Microsoft::WRL::ComPtr<IDxcBlob> blob;
};

class DX12ShaderCompiler
{
private:
	Microsoft::WRL::ComPtr<IDxcLibrary> m_library;
	Microsoft::WRL::ComPtr<IDxcUtils> m_utils;
	Microsoft::WRL::ComPtr<IDxcCompiler> m_compiler;
	Microsoft::WRL::ComPtr<IDxcIncludeHandler> m_defIncHandler;

public:
	DX12ShaderCompiler();
	~DX12ShaderCompiler();

	CompiledShader CompileShader(const std::wstring& shader_path, const ShaderType& shader_type);
};

