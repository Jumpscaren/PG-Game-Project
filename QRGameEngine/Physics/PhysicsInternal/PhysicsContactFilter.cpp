#include "pch.h"
#include "PhysicsContactFilter.h"

bool PhysicsContactFilter::ShouldCollide(b2Fixture* fixtureA, b2Fixture* fixtureB)
{
    const auto filterA_data = fixtureA->GetFilterData();
    const auto filterB_data = fixtureB->GetFilterData();
    bool collide =
        (filterA_data.maskBits & filterB_data.categoryBits) != 0 &&
        (filterA_data.categoryBits & filterB_data.maskBits) != 0;

    return collide;
}
