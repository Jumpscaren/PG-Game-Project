#pragma once
#include "TransformComponent.h"

struct ParentComponent : public TransformComponent
{
	Entity parent;
};

