#include "pch.h"
#include "EngineTypes.h"

Vector2::Vector2(float x, float y) : x(x), y(y)
{
	
}

Vector2::Vector2(const DirectX::XMVECTOR& vector)
{
	x = vector.m128_f32[0]; 
	y = vector.m128_f32[1];
}

Vector2::operator DirectX::XMVECTOR() const
{
	DirectX::XMVECTOR vector;

	vector.m128_f32[0] = x;
	vector.m128_f32[1] = y;

	return vector;
}

const Vector2 Vector2::operator=(const DirectX::XMVECTOR& vector)
{
	return Vector2(vector.m128_f32[0], vector.m128_f32[1]);
}

Vector3::Vector3(float x, float y, float z) : x(x), y(y), z(z)
{
	
}

Vector3::Vector3(const DirectX::XMVECTOR& vector)
{
	x = vector.m128_f32[0];
	y = vector.m128_f32[1];
	z = vector.m128_f32[2];
}

Vector3::operator DirectX::XMVECTOR() const
{
	DirectX::XMVECTOR vector;

	vector.m128_f32[0] = x;
	vector.m128_f32[1] = y;
	vector.m128_f32[2] = z;

	return vector;
}

const Vector3 Vector3::operator=(const DirectX::XMVECTOR& vector)
{
	return Vector3(vector.m128_f32[0], vector.m128_f32[1], vector.m128_f32[2]);
}

Vector4::Vector4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w)
{
	
}

Vector4::Vector4(const DirectX::XMVECTOR& vector)
{
	x = vector.m128_f32[0];
	y = vector.m128_f32[1];
	z = vector.m128_f32[2];
	w = vector.m128_f32[3];
}

Vector4::operator DirectX::XMVECTOR() const
{
	DirectX::XMVECTOR vector;

	vector.m128_f32[0] = x;
	vector.m128_f32[1] = y;
	vector.m128_f32[2] = z;
	vector.m128_f32[3] = w;

	return vector;
}

const Vector4 Vector4::operator=(const DirectX::XMVECTOR& vector)
{
	return Vector4(vector.m128_f32[0], vector.m128_f32[1], vector.m128_f32[2], vector.m128_f32[3]);
}

Vector2i::Vector2i(int32_t x, int32_t y) : x(x), y(y)
{

}

Vector2u::Vector2u(uint32_t x, uint32_t y) : x(x), y(y)
{
}
