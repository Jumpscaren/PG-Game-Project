#pragma once
#include "Vendor/Include/Box2D/box2d.h"
#include "ECS/EntityDefinition.h"
#include "SceneSystem/SceneDefines.h"
#include "Common/EngineTypes.h"

class PhysicsContactFilter : public b2ContactFilter
{
public:
	PhysicsContactFilter() {};

	bool ShouldCollide(b2Fixture* fixtureA, b2Fixture* fixtureB) override;
};

