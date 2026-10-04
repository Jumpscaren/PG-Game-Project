#pragma once
#include <string>

class IMaterial
{
public:
	virtual ~IMaterial() = default;

	virtual void SetVertexShader(const std::wstring& vertex_shader_path) = 0;
	virtual void SetPixelShader(const std::wstring& pixel_shader_path) = 0;
};