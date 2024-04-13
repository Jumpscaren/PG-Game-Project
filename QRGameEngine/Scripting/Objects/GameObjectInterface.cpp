#include "pch.h"
#include "GameObjectInterface.h"
#include "Scripting/CSMonoCore.h"
#include "Scripting/Objects/SceneInterface.h"
#include "SceneSystem/SceneManager.h"
#include "ECS/EntityManager.h"
#include "Components/EntityDataComponent.h"
#include "SceneSystem/SceneHierarchy.h"

MonoFieldHandle GameObjectInterface::get_entity_id_field;
MonoClassHandle GameObjectInterface::game_object_class;
MonoMethodHandle GameObjectInterface::get_scene_index_method;
MonoMethodHandle GameObjectInterface::create_game_object_method;
MonoMethodHandle GameObjectInterface::new_game_object_with_existing_entity_method;

void GameObjectInterface::RegisterInterface(CSMonoCore* mono_core)
{
    game_object_class = mono_core->RegisterMonoClass("ScriptProject.Engine", "GameObject");
    get_scene_index_method = mono_core->RegisterMonoMethod(game_object_class, "GetSceneIndex");
    get_entity_id_field = mono_core->RegisterField(game_object_class, "entity_id");
    create_game_object_method = mono_core->RegisterMonoMethod(game_object_class, "CreateGameObject");
    new_game_object_with_existing_entity_method = mono_core->RegisterMonoMethod(game_object_class, "NewGameObjectWithExistingEntity");

    mono_core->HookAndRegisterMonoMethodType<GameObjectInterface::AddEntityData>(game_object_class, "AddEntityData", GameObjectInterface::AddEntityData);
    mono_core->HookAndRegisterMonoMethodType<GameObjectInterface::SetName>(game_object_class, "SetName", GameObjectInterface::SetName);
    mono_core->HookAndRegisterMonoMethodType<GameObjectInterface::GetName>(game_object_class, "GetName", GameObjectInterface::GetName);
    mono_core->HookAndRegisterMonoMethodType<GameObjectInterface::TempFindGameObject>(game_object_class, "TempFindGameObject", GameObjectInterface::TempFindGameObject);

    mono_core->HookAndRegisterMonoMethodType<GameObjectInterface::AddChild>(game_object_class, "AddChild", GameObjectInterface::AddChild);
    mono_core->HookAndRegisterMonoMethodType<GameObjectInterface::RemoveChild>(game_object_class, "RemoveChild", GameObjectInterface::RemoveChild);
    mono_core->HookAndRegisterMonoMethodType<GameObjectInterface::HasChildren>(game_object_class, "HasChildren", GameObjectInterface::HasChildren);
    mono_core->HookAndRegisterMonoMethodType<GameObjectInterface::DestroyChildren>(game_object_class, "DestroyChildren", GameObjectInterface::DestroyChildren);
}

CSMonoObject GameObjectInterface::GetGameObjectFromComponent(const CSMonoObject& component)
{
    CSMonoObject game_object;
    CSMonoCore::Get()->GetValue(game_object, component, "game_object");
    return game_object;
}

Entity GameObjectInterface::GetEntityID(const CSMonoObject& game_object)
{
    Entity entity_id;
    CSMonoCore::Get()->GetValue(entity_id, game_object, get_entity_id_field);
    return entity_id;
}

SceneIndex GameObjectInterface::GetSceneIndex(const CSMonoObject& game_object)
{
    SceneIndex scene_index;
    CSMonoCore::Get()->CallMethod(scene_index, get_scene_index_method, game_object);
    return scene_index;
}

CSMonoObject GameObjectInterface::CreateGameObject()
{
    CSMonoObject game_object;
    CSMonoCore::Get()->CallStaticMethod(game_object, create_game_object_method);

    return game_object;
}

CSMonoObject GameObjectInterface::NewGameObjectWithExistingEntity(Entity entity, SceneIndex scene_index)
{
    CSMonoObject game_object;
    CSMonoObject scene_object = SceneInterface::CreateSceneWithSceneIndex(scene_index);
    CSMonoCore::Get()->CallStaticMethod(game_object, new_game_object_with_existing_entity_method, entity, scene_object);

    return game_object;
}

void GameObjectInterface::AddEntityData(CSMonoObject object)
{
    if (!SceneManager::GetSceneManager()->GetEntityManager(GetSceneIndex(object))->HasComponent<EntityDataComponent>(GetEntityID(object)))
        SceneManager::GetSceneManager()->GetEntityManager(GetSceneIndex(object))->AddComponent<EntityDataComponent>(GetEntityID(object));
}

void GameObjectInterface::SetName(CSMonoObject object, std::string name)
{
    SceneManager::GetSceneManager()->GetEntityManager(GetSceneIndex(object))->GetComponent<EntityDataComponent>(GetEntityID(object)).entity_name = name;
}

std::string GameObjectInterface::GetName(CSMonoObject object)
{
    return SceneManager::GetSceneManager()->GetEntityManager(GetSceneIndex(object))->GetComponent<EntityDataComponent>(GetEntityID(object)).entity_name;
}

CSMonoObject GameObjectInterface::TempFindGameObject(std::string name)
{
    Entity found_game_object = NULL_ENTITY;
    SceneManager::GetSceneManager()->GetEntityManager(SceneManager::GetSceneManager()->GetActiveSceneIndex())->System<EntityDataComponent>([&](const Entity entity, const EntityDataComponent& entity_data)
        {
            if (found_game_object != NULL_ENTITY)
                return;

            const std::string entity_name = entity_data.entity_name.substr(0, name.length());
            
            if (entity_name == name)
                found_game_object = entity;
        });
    return NewGameObjectWithExistingEntity(found_game_object, SceneManager::GetSceneManager()->GetActiveSceneIndex());
}

Entity GameObjectInterface::TempFindGameObjectEntity(const std::string& name)
{
    Entity found_game_object = NULL_ENTITY;
    SceneManager::GetSceneManager()->GetEntityManager(SceneManager::GetSceneManager()->GetActiveSceneIndex())->System<EntityDataComponent>([&](const Entity entity, const EntityDataComponent& entity_data)
        {
            if (found_game_object != NULL_ENTITY)
                return;

            const std::string entity_name = entity_data.entity_name.substr(0, name.length());

            if (entity_name == name)
                found_game_object = entity;
        });
    return found_game_object;
}

void GameObjectInterface::AddChild(const CSMonoObject game_object, const CSMonoObject child_game_object)
{
    const auto scene_index = GameObjectInterface::GetSceneIndex(game_object);
    const auto entity = GameObjectInterface::GetEntityID(game_object);
    const auto child_entity = GameObjectInterface::GetEntityID(child_game_object);

    SceneHierarchy::Get()->AddParentChildRelation(scene_index, entity, child_entity);
}

void GameObjectInterface::RemoveChild(const CSMonoObject game_object, const CSMonoObject child_game_object)
{
    const auto scene_index = GameObjectInterface::GetSceneIndex(game_object);
    const auto child_entity = GameObjectInterface::GetEntityID(child_game_object);

    SceneHierarchy::Get()->RemoveParentChildRelation(scene_index, child_entity);
}

bool GameObjectInterface::HasChildren(const CSMonoObject game_object)
{
    const auto scene_index = GameObjectInterface::GetSceneIndex(game_object);
    const auto parent_entity = GameObjectInterface::GetEntityID(game_object);

    return SceneHierarchy::Get()->ParentHasChildren(scene_index, parent_entity);
}

void GameObjectInterface::DestroyChildren(const CSMonoObject game_object)
{
    const auto scene_index = GameObjectInterface::GetSceneIndex(game_object);
    const auto parent_entity = GameObjectInterface::GetEntityID(game_object);
	EntityManager* entity_manager = SceneManager::GetSceneManager()->GetEntityManager(scene_index);
	const auto& children = SceneHierarchy::Get()->GetAllChildren(scene_index, parent_entity);
	for (const auto entity : children)
	{
		entity_manager->RemoveEntity(entity);
	}
}
