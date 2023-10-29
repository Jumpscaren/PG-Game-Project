#include "pch.h"
#include "EntityDataComponent.h"
#include "ECS/EntityManager.h"
#include "IO/JsonObject.h"
#include "SceneSystem/SceneLoader.h"

void EntityDataComponentInterface::RegisterInterface(CSMonoCore*)
{
	SceneLoader::Get()->OverrideSaveComponentMethod<EntityDataComponent>(SaveScriptComponent, LoadScriptComponent);
}

void EntityDataComponentInterface::SaveScriptComponent(Entity ent, EntityManager* entman, JsonObject* json_object)
{
	const EntityDataComponent& entity_data = entman->GetComponent<EntityDataComponent>(ent);
	json_object->SetData(entity_data.entity_name, "entity_name");
	json_object->SetData(entity_data.entity_tag, "entity_tag");
	json_object->SetData(entity_data.entity_layer, "entity_layer");
}

void EntityDataComponentInterface::LoadScriptComponent(Entity ent, EntityManager* entman, JsonObject* json_object)
{
	EntityDataComponent& entity_data = entman->GetComponent<EntityDataComponent>(ent);
	json_object->LoadData(entity_data.entity_name, "entity_name");
	json_object->LoadData(entity_data.entity_tag, "entity_tag");
	json_object->LoadData(entity_data.entity_layer, "entity_layer");
}
