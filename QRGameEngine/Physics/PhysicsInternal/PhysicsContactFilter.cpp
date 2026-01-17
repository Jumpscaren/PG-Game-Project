#include "pch.h"
#include "PhysicsContactFilter.h"
#include "Vendor/Include/Box2D/IncludeBox2D.h"

bool PhysicsContactFilter::ShouldCollide(b2ShapeId shape_a, b2ShapeId shape_b)
{
    const auto filterA_data = b2Shape_GetFilter(shape_a);
    const auto filterB_data = b2Shape_GetFilter(shape_b);
    const bool collide =
        (filterA_data.maskBits & filterB_data.categoryBits) != 0 &&
        (filterA_data.categoryBits & filterB_data.maskBits) != 0;

    return collide;
}

bool PhysicsContactFilter::ShouldCollide(b2ShapeId shape, const ColliderFilter collider_filter)
{
    const auto filter_data = b2Shape_GetFilter(shape);
    const bool collide =
        (filter_data.maskBits & collider_filter.category_bits) != 0 &&
        (filter_data.categoryBits & collider_filter.mask_bits) != 0;

    return collide;
}
