#pragma once
#include "Common/EngineTypes.h"

class PhysicsDebugDraw
{
public:
	/// Draw a closed polygon provided in CCW order.
	void DrawPolygon(const std::vector<Vector2>& vertices, const Vector3& color);

	/// Draw a solid closed polygon provided in CCW order.
	void DrawSolidPolygon(const std::vector<Vector2>& vertices, const Vector3& color);

	/// Draw a circle.
	void DrawCircle(const Vector2& center, float radius, const Vector3& color);

	/// Draw a solid circle.
	void DrawSolidCircle(const Vector2& center, float radius, const Vector2& axis, const Vector3& color);

	/// Draw a line segment.
	void DrawSegment(const Vector2& p1, const Vector2& p2, const Vector3& color);

private:
	static constexpr float PI = 3.14159265359f;
};

