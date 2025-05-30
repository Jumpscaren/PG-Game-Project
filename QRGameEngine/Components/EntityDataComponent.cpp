#include "pch.h"
#include "EntityDataComponent.h"
#include "ECS/EntityManager.h"
#include "IO/JsonObject.h"
#include "SceneSystem/SceneLoader.h"

void EntityDataComponentInterface::RegisterInterface(CSMonoCore*)
{
	SceneLoader::Get()->OverrideSaveComponentMethod<EntityDataComponent>(SaveEntityDataComponent, LoadEntityDataComponent);
}

void EntityDataComponentInterface::SaveEntityDataComponent(Entity ent, EntityManager* entman, JsonObject* json_object)
{
	const EntityDataComponent& entity_data = entman->GetComponent<EntityDataComponent>(ent);
	json_object->SetData(entity_data.entity_name, "entity_name");
	json_object->SetData(entity_data.entity_tag, "entity_tag");
	json_object->SetData(entity_data.entity_layer, "entity_layer");
}

void EntityDataComponentInterface::LoadEntityDataComponent(Entity ent, EntityManager* entman, JsonObject* json_object)
{
	EntityDataComponent& entity_data = entman->GetComponent<EntityDataComponent>(ent);
	json_object->LoadData(entity_data.entity_name, "entity_name");
	json_object->LoadData(entity_data.entity_tag, "entity_tag");
	json_object->LoadData(entity_data.entity_layer, "entity_layer");

	entity_data.entity_name.erase(std::remove(entity_data.entity_name.begin(), entity_data.entity_name.end(), 0), entity_data.entity_name.end());
}
