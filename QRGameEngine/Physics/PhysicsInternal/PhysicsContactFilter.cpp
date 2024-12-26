#include "pch.h"
#include "PhysicsContactFilter.h"

bool PhysicsContactFilter::ShouldCollide(b2Fixture* fixtureA, b2Fixture* fixtureB)
{
    const auto filterA_data = fixtureA->GetFilterData();
    const auto filterB_data = fixtureB->GetFilterData();
    const bool collide =
        (filterA_data.maskBits & filterB_data.categoryBits) != 0 &&
        (filterA_data.categoryBits & filterB_data.maskBits) != 0;

    return collide;
}

bool PhysicsContactFilter::ShouldCollide(b2Fixture* fixture, const ColliderFilter collider_filter)
{
    const auto filter_data = fixture->GetFilterData();
    const bool collide =
        (filter_data.maskBits & collider_filter.category_bits) != 0 &&
        (filter_data.categoryBits & collider_filter.mask_bits) != 0;

    return collide;
}
