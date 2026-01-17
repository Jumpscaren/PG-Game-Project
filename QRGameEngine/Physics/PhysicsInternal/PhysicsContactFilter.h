#pragma once
#include "Vendor/Include/Box2D/id.h"
#include "../PhysicDefines.h"

class PhysicsContactFilter
{
public:
	PhysicsContactFilter() {};

	static bool ShouldCollide(b2ShapeId shape_a, b2ShapeId shape_b);
	
	static bool ShouldCollide(b2ShapeId shape, const ColliderFilter collider_filter);
};

