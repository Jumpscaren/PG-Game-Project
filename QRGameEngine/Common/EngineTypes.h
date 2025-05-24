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
	friend const Vector2 operator* (const Vector2& vector_1, float scalar);
	const Vector2 operator*= (const float scalar);
	const Vector2 operator* (float scalar);
	const Vector2 operator/ (float scalar);

	Vector2 Normalize() const;
	const float Length() const;
	static float Cross(const Vector2& a, const Vector2& b);
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
	const Vector3 operator+ (const Vector3& vector);
	friend const Vector3 operator+ (const Vector3& vector_1, const Vector3& vector_2);
	const Vector3 operator- (const Vector3& vector);
	friend const Vector3 operator- (const Vector3& vector_1, const Vector3& vector_2);
	const Vector3 operator* (float scalar);

	Vector3 Normalize() const;
	const float Length() const;

	static float Dot(const Vector3& a, const Vector3& b);
	static Vector3 Cross(const Vector3& a, const Vector3& b);

	static Vector3 ToVector3(const Vector2& vector);
};

struct Vector4
{
	float x, y, z, w;

	Vector4(float x = 0, float y = 0, float z = 0, float w = 0);
	Vector4(const DirectX::XMVECTOR& vector);

	operator DirectX::XMVECTOR() const;
	const Vector4 operator= (const DirectX::XMVECTOR& vector);
};