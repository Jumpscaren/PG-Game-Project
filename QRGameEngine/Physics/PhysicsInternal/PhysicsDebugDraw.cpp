#include "pch.h"
#include "PhysicsDebugDraw.h"
#include "Renderer/RenderCore.h"

/// Draw a closed polygon provided in CCW order.
void PhysicsDebugDraw::DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color)
{
	b2Vec2 p1 = vertices[vertexCount - 1];
	for (int32 i = 0; i < vertexCount; ++i)
	{
		b2Vec2 p2 = vertices[i];
		RenderCore::Get()->AddLine(Vector2(p1.x, p1.y));
		RenderCore::Get()->AddLine(Vector2(p2.x, p2.y));
		p1 = p2;
	}
}

/// Draw a solid closed polygon provided in CCW order.
void PhysicsDebugDraw::DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color)
{
	b2Vec2 p1 = vertices[vertexCount - 1];
	for (int32 i = 0; i < vertexCount; ++i)
	{
		b2Vec2 p2 = vertices[i];
		RenderCore::Get()->AddLine(Vector2(p1.x, p1.y));
		RenderCore::Get()->AddLine(Vector2(p2.x, p2.y));
		p1 = p2;
	}
}

/// Draw a circle.
void PhysicsDebugDraw::DrawCircle(const b2Vec2& center, float radius, const b2Color& color)
{
	const float k_segments = 16.0f;
	const float k_increment = 2.0f * b2_pi / k_segments;
	float sinInc = sinf(k_increment);
	float cosInc = cosf(k_increment);
	b2Vec2 r1(1.0f, 0.0f);
	b2Vec2 v1 = center + radius * r1;
	for (int32 i = 0; i < k_segments; ++i)
	{
		// Perform rotation to avoid additional trigonometry.
		b2Vec2 r2;
		r2.x = cosInc * r1.x - sinInc * r1.y;
		r2.y = sinInc * r1.x + cosInc * r1.y;
		b2Vec2 v2 = center + radius * r2;
		//m_lines->Vertex(v1, color);
		//m_lines->Vertex(v2, color);
		RenderCore::Get()->AddLine(Vector2(v1.x, v1.y));
		RenderCore::Get()->AddLine(Vector2(v2.x, v2.y));
		r1 = r2;
		v1 = v2;
	}
}

/// Draw a solid circle.
void PhysicsDebugDraw::DrawSolidCircle(const b2Vec2& center, float radius, const b2Vec2& axis, const b2Color& color)
{
	const float k_segments = 16.0f;
	const float k_increment = 2.0f * b2_pi / k_segments;
	const float sinInc = sinf(k_increment);
	const float cosInc = cosf(k_increment);
	b2Vec2 r1(1.0f, 0.0f);
	b2Vec2 v1 = center + radius * r1;
	for (int32 i = 0; i < k_segments; ++i)
	{
		// Perform rotation to avoid additional trigonometry.
		b2Vec2 r2;
		r2.x = cosInc * r1.x - sinInc * r1.y;
		r2.y = sinInc * r1.x + cosInc * r1.y;
		b2Vec2 v2 = center + radius * r2;
		RenderCore::Get()->AddLine(Vector2(v1.x, v1.y));
		RenderCore::Get()->AddLine(Vector2(v2.x, v2.y));
		r1 = r2;
		v1 = v2;
	}

	// Draw a line fixed in the circle to animate rotation.
	b2Vec2 p = center + radius * axis;
	RenderCore::Get()->AddLine(Vector2(center.x, center.y));
	RenderCore::Get()->AddLine(Vector2(p.x, p.y));
}

/// Draw a line segment.
void PhysicsDebugDraw::DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color)
{

}

/// Draw a transform. Choose your own length scale.
/// @param xf a transform.
void PhysicsDebugDraw::DrawTransform(const b2Transform& xf)
{

}

/// Draw a point.
void PhysicsDebugDraw::DrawPoint(const b2Vec2& p, float size, const b2Color& color)
{

}