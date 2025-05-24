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

const Vector2 Vector2::operator+(const Vector2& vector)
{
	return Vector2(x + vector.x, y + vector.y);
}

const Vector2 Vector2::operator-(const Vector2& vector)
{
	return Vector2(x - vector.x, y - vector.y);
}

const Vector2 operator+(const Vector2& vector_1, const Vector2& vector_2)
{
	return Vector2(vector_1.x + vector_2.x, vector_1.y + vector_2.y);
}

const Vector2 operator-(const Vector2& vector_1, const Vector2& vector_2)
{
	return Vector2(vector_1.x - vector_2.x, vector_1.y - vector_2.y);
}

const Vector2 operator*(const Vector2& vector_1, float scalar)
{
	return Vector2(vector_1.x * scalar, vector_1.y * scalar);
}

const Vector2 Vector2::operator*=(float scalar)
{
	return Vector2(x * scalar, y * scalar);
}

const Vector2 Vector2::operator*(const float scalar)
{
	return Vector2(x * scalar, y * scalar);
}

const Vector2 Vector2::operator/(const float scalar)
{
	return Vector2(x / scalar, y / scalar);
}

Vector2 Vector2::Normalize() const
{
	float length = Length();
	if (length < 1e-05f)
		return Vector2();
	return Vector2(x, y) / length;
}

const float Vector2::Length() const
{
	return std::sqrt(x*x + y*y);
}

float Vector2::Cross(const Vector2& a, const Vector2& b)
{
	return a.x*b.y - b.x*a.y;
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

const Vector3 Vector3::operator+(const Vector3& vector)
{
	return Vector3(x + vector.x, y + vector.y, z + vector.z);
}

const Vector3 Vector3::operator-(const Vector3& vector)
{
	return Vector3(x - vector.x, y - vector.y, z - vector.z);
}

const Vector3 operator+(const Vector3& vector_1, const Vector3& vector_2)
{
	return Vector3(vector_1.x + vector_2.x, vector_1.y + vector_2.y, vector_1.z + vector_2.z);
}

const Vector3 operator-(const Vector3& vector_1, const Vector3& vector_2)
{
	return Vector3(vector_1.x - vector_2.x, vector_1.y - vector_2.y, vector_1.z - vector_2.z);
}

const Vector3 Vector3::operator*(float scalar)
{
	return Vector3(x * scalar, y * scalar, z * scalar);
}

Vector3 Vector3::Normalize() const
{
	float length = Length();
	if (length < 1e-05f)
		return Vector3();
	return Vector3(x / length, y / length, z / length);
}

const float Vector3::Length() const
{
	return std::sqrt(x * x + y * y + z * z);
}

float Vector3::Dot(const Vector3& a, const Vector3& b)
{
	return a.x*b.x + a.y*b.y;
}

Vector3 Vector3::Cross(const Vector3& a, const Vector3& b)
{
	return Vector3(a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x);
}

Vector3 Vector3::ToVector3(const Vector2& vector)
{
	return Vector3(vector.x, vector.y);
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
