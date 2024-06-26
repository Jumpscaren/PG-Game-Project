#pragma once

struct Vector2
{
	float x, y;

	Vector2(float x = 0, float y = 0);
	Vector2(const DirectX::XMVECTOR& vector);

	operator DirectX::XMVECTOR() const;
	const Vector2 operator= (const DirectX::XMVECTOR& vector);
	const Vector2 operator+ (const Vector2& vector);
	friend const Vector2 operator+ (const Vector2& vector_1, const Vector2& vector_2);
	const Vector2 operator- (const Vector2& vector);
	friend const Vector2 operator- (const Vector2& vector_1, const Vector2& vector_2);
	const Vector2 operator* (const Vector2& vector);
	friend const Vector2 operator* (const Vector2& vector_1, const Vector2& vector_2);
	const Vector2 operator*= (const Vector2& vector);
	const Vector2 operator* (uint8_t scalar);
	const Vector2 operator/ (float scalar);

	Vector2 Normalize() const;
	const float Length() const;
};

struct Vector2i
{
	int32_t x, y;

	Vector2i(int32_t x = 0, int32_t y = 0);
};

struct Vector2u
{
	uint32_t x, y;

	Vector2u(uint32_t x = 0, uint32_t y = 0);
};

struct Vector3
{
	float x, y, z;

	Vector3(float x = 0, float y = 0, float z = 0);
	Vector3(const DirectX::XMVECTOR& vector);

	operator DirectX::XMVECTOR() const;
	const Vector3 operator= (const DirectX::XMVECTOR& vector);
	const Vector3 operator* (float scalar);
};

struct Vector4
{
	float x, y, z, w;

	Vector4(float x = 0, float y = 0, float z = 0, float w = 0);
	Vector4(const DirectX::XMVECTOR& vector);

	operator DirectX::XMVECTOR() const;
	const Vector4 operator= (const DirectX::XMVECTOR& vector);
};