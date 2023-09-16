#pragma once
#include "Vendor/Include/Box2D/box2d.h"

class PhysicsDestructionListener : public b2DestructionListener
{
	void SayGoodbye(b2Fixture* fixture) override;
	void SayGoodbye(b2Joint*) override;
};

