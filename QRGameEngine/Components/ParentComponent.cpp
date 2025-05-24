#include "pch.h"
#include "ParentComponent.h"
#include "Scripting/CSMonoCore.h"
#include "SceneSystem/SceneLoader.h"
#include "IO/JsonObject.h"

void ParentComponentInterface::RegisterInterface(CSMonoCore* mono_core)
{
	SceneLoader::Get()->OverrideSaveComponentMethod<ParentComponent>(SaveParentComponent, LoadParentComponent);
}

void ParentComponentInterface::SaveParentComponent(Entity ent, EntityManager* entman, JsonObject* json_object)
{
	TransformComponentInterface::SaveTransformComponent(ent, entman, json_object);
}

void ParentComponentInterface::LoadParentComponent(Entity ent, EntityManager* entman, JsonObject* json_object)
{
	TransformComponentInterface::LoadTransformComponent(ent, entman, json_object);
}
