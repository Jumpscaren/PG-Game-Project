#include "pch.h"
#include "PhysicsDebugDraw.h"
#include "Renderer/RenderCore.h"

/// Draw a closed polygon provided in CCW order.
void PhysicsDebugDraw::DrawPolygon(const std::vector<Vector2>& vertices, const Vector3& color)
{
	Vector2 p1 = vertices[vertices.size() - 1];
	for (int32_t i = 0; i < vertices.size(); ++i)
	{
		const Vector2& p2 = vertices[i];
		RenderCore::Get()->AddLine(p1);
		RenderCore::Get()->AddLine(p2);
		p1 = p2;
	}
}

/// Draw a solid closed polygon provided in CCW order.
void PhysicsDebugDraw::DrawSolidPolygon(const std::vector<Vector2>& vertices, const Vector3& color)
{
	Vector2 p1 = vertices[vertices.size() - 1];
	for (int32_t i = 0; i < vertices.size(); ++i)
	{
		const Vector2& p2 = vertices[i];
		RenderCore::Get()->AddLine(p1);
		RenderCore::Get()->AddLine(p2);
		p1 = p2;
	}
}

/// Draw a circle.
void PhysicsDebugDraw::DrawCircle(const Vector2& center, float radius, const Vector3& color)
{
	const float k_segments = 16.0f;
	const float k_increment = 2.0f * PI / k_segments;
	float sinInc = sinf(k_increment);
	float cosInc = cosf(k_increment);
	Vector2 r1(1.0f, 0.0f);
	Vector2 v1 = center + radius * r1;
	for (int32_t i = 0; i < k_segments; ++i)
	{
		// Perform rotation to avoid additional trigonometry.
		Vector2 r2;
		r2.x = cosInc * r1.x - sinInc * r1.y;
		r2.y = sinInc * r1.x + cosInc * r1.y;
		Vector2 v2 = center + radius * r2;
		RenderCore::Get()->AddLine(v1);
		RenderCore::Get()->AddLine(v2);
		r1 = r2;
		v1 = v2;
	}
}

/// Draw a solid circle.
void PhysicsDebugDraw::DrawSolidCircle(const Vector2& center, float radius, const Vector2& axis, const Vector3& color)
{
	const float k_segments = 16.0f;
	const float k_increment = 2.0f * PI / k_segments;
	const float sinInc = sinf(k_increment);
	const float cosInc = cosf(k_increment);
	Vector2 r1(1.0f, 0.0f);
	Vector2 v1 = center + radius * r1;
	for (int32_t i = 0; i < k_segments; ++i)
	{
		// Perform rotation to avoid additional trigonometry.
		Vector2 r2;
		r2.x = cosInc * r1.x - sinInc * r1.y;
		r2.y = sinInc * r1.x + cosInc * r1.y;
		Vector2 v2 = center + radius * r2;
		RenderCore::Get()->AddLine(Vector2(v1.x, v1.y));
		RenderCore::Get()->AddLine(Vector2(v2.x, v2.y));
		r1 = r2;
		v1 = v2;
	}

	// Draw a line fixed in the circle to animate rotation.
	Vector2 p = center + radius * axis;
	RenderCore::Get()->AddLine(center);
	RenderCore::Get()->AddLine(p);
}

/// Draw a line segment.
void PhysicsDebugDraw::DrawSegment(const Vector2& p1, const Vector2& p2, const Vector3& color)
{
	RenderCore::Get()->AddLine(Vector2(p1.x, p1.y));
	RenderCore::Get()->AddLine(Vector2(p2.x, p2.y));
}