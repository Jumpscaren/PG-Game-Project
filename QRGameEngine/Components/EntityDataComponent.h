#pragma once
#include "Physics/PhysicDefines.h"
#include "Scripting/CSMonoObject.h"
#include "ECS/EntityDefinition.h"
#include "SceneSystem/SceneDefines.h"
#include "Common/EngineTypes.h"


struct EntityDataComponent
{
	std::string entity_name;
	uint8_t entity_tag;
	uint8_t entity_layer;
};


class JsonObject;
class EntityManager;

class EntityDataComponentInterface
{
public:
	static void RegisterInterface(CSMonoCore* mono_core);

public:
	static void SaveScriptComponent(Entity ent, EntityManager* entman, JsonObject* json_object);
	static void LoadScriptComponent(Entity ent, EntityManager* entman, JsonObject* json_object);
};
