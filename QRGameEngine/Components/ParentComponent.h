#pragma once
#include "TransformComponent.h"

struct ParentComponent : public TransformComponent
{
	Entity parent;
};

class JsonObject;
class EntityManager;

class ParentComponentInterface
{
public:
	static void RegisterInterface(CSMonoCore* mono_core);

public:
	static void SaveParentComponent(Entity ent, EntityManager* entman, JsonObject* json_object);
	static void LoadParentComponent(Entity ent, EntityManager* entman, JsonObject* json_object);
};