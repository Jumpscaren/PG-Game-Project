#include "pch.h"
#include "PhysicsDestructionListener.h"

void PhysicsDestructionListener::SayGoodbye(b2Fixture* fixture)
{
	{ B2_NOT_USED(fixture); }
}

void PhysicsDestructionListener::SayGoodbye(b2Joint*)
{
}
